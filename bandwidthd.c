#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include "bandwidthd.h"


// We must call regular exit to write out profile data, but child forks are supposed to usually
// call _exit?
#ifdef PROFILE
#define _exit(x) exit(x)
#endif

/*
#ifdef DEBUG
#define fork() (0)
#endif
*/

// ****************************************************************************************
// ** Global Variables
// ****************************************************************************************

static pcap_t *pd;

unsigned int GraphIntervalCount = 0;
unsigned int IpCount = 0;
unsigned int SubnetCount = 0;
time_t IntervalStart;
int RotateLogs = FALSE;
    
struct SubnetData SubnetTable[SUBNET_NUM];
struct IPData IpTable[IP_NUM];

key_t IPCKey;
int shmid;
int DataLink;
int IP_Offset;

struct IPCData *IPCSharedData;

struct IPDataStore *IPDataStore = NULL;
extern int yyparse(void);
extern FILE *yyin;

struct config config;

void signal_handler(int sig)
	{
	signal(SIGHUP, signal_handler);
	RotateLogs = TRUE; 	
	}

void bd_CollectingData(char *filename)
	{
	char CurrentDirWarning[] = "bandwidthd always works out of the current directory, cd to some place with a ./etc/bandwidthd.conf and a ./htdocs/ then run it";
	FILE *index;

	index = fopen(filename, "w");	
	if (index)
		{
		fprintf(index, "<HTML><HEAD>\n<META HTTP-EQUIV=\"REFRESH\" content=\"150\">\n<META HTTP-EQUIV=\"EXPIRES\" content=\"-1\">\n");
		fprintf(index, "<META HTTP-EQUIV=\"PRAGMA\" content=\"no-cache\">\n");
		fprintf(index, "</HEAD><BODY><center><img src=\"logo.gif\"><BR>\n");
		fprintf(index, "<BR>\n - <a href=index.html>Daily</a> -- <a href=index2.html>Weekly</a> -- ");
		fprintf(index, "<a href=index3.html>Monthly</a> -- <a href=index4.html>Yearly</a><BR>\n");
		fprintf(index, "<BR>bandwidthd is collecting data...\n");		
		fprintf(index, "</BODY></HTML>\n");
		fclose(index);
		}
	else
		{
		printf("Cannot open %s: %s\n", filename, CurrentDirWarning);
		exit(1);
		}
	}

void WriteOutWebpages(long int timestamp)
	{
    struct IPDataStore *DataStore;
	int NumGraphs = 0;

	// break off from the main line so we don't miss any packets while we graph
	DataStore = IPDataStore;
	if (DataStore)
		{
		if (!fork()) // if there is a datastore to graph fork, and if we're the child, graph it.
			{
#ifdef PROFILE
			// Got this incantation from a message board.  Don't forget to set
			// GMON_OUT_PREFIX in the shell
			extern void _start(void), etext(void);
			printf("Calling profiler startup...\n");
			monstartup((u_long) &_start, (u_long) &etext);
#endif
          	signal(SIGHUP, SIG_IGN);

     	    nice(4); // reduce priority so I don't choke out other tasks
			
			while (DataStore) // Is not null
				{
				if (DataStore->FirstBlock->NumEntries > 0)
					GraphIp(DataStore, &IPCSharedData[NumGraphs++], timestamp+LEAD*config.range);
			    DataStore = DataStore->Next;
				}

			MakeIndexPages(NumGraphs);
	
			_exit(0);
			}
		}	
	}

int main(int argc, char **argv)
    {
    struct bpf_program fcode;
    u_char *pcap_userdata = 0;
    struct shmid_ds shmstatus;
	char Error[PCAP_ERRBUF_SIZE];
	char CurrentDirWarning[] = "bandwidthd always works out of the current directory, cd to some place with a ./etc/bandwidthd.conf and a ./htdocs/ then run it";
	unsigned long long graph_cutoff;

	config.dev = NULL;
	config.filter = "ip";
	config.skip_intervals = CONFIG_GRAPHINTERVALS;
	config.graph_cutoff = CONFIG_GRAPHCUTOFF;
	config.promisc = TRUE;
	config.graph = TRUE;
	config.output_cdf = FALSE;
	config.recover_cdf = FALSE;

  
	yyin = fopen("./etc/bandwidthd.conf", "r");
	if (!yyin)
		{
		printf("Cannot open ./etc/bandwidthd.conf: %s\n", CurrentDirWarning);
		exit(1);
		}
	yyparse();
	
	// Scary
	//printf("Max ram utilization is %dMBytes.\n", (int)((sizeof(struct IPData)*(config.range/180)*IP_NUM)/1024)/1024);

	bd_CollectingData("htdocs/index.html");
	bd_CollectingData("htdocs/index2.html");
	bd_CollectingData("htdocs/index3.html");
	bd_CollectingData("htdocs/index4.html");

	if (fork2())
		exit(0);

	config.range = RANGE1;
	config.interval = INTERVAL1;
	config.tag = '1';
	graph_cutoff = config.graph_cutoff;

	if (fork())  // Fork into seperate process for day, week, and month
		{
		config.skip_intervals = CONFIG_GRAPHINTERVALS; // Overide skip_intervals for children
		config.range = RANGE2;
		config.interval = INTERVAL2;
		config.tag = '2';
		config.graph_cutoff = graph_cutoff*(RANGE2/RANGE1);	
		if (fork())
			{
			config.range = RANGE3;
			config.interval = INTERVAL3;
			config.tag = '3';
			config.graph_cutoff = graph_cutoff*(RANGE3/RANGE1);
			if (fork())
				{
	            config.range = RANGE4;
    	        config.interval = INTERVAL4;
        	    config.tag = '4';
				config.graph_cutoff = graph_cutoff*(RANGE4/RANGE1);
            	if (fork())
                	exit(0);
				}
			}
		}
	
    if(config.recover_cdf)
	    RecoverDataFromCDF();
	
	IPCKey = ftok(".", config.tag);
	if ((shmid = shmget(IPCKey, sizeof(struct IPCData)*IP_NUM, IPC_CREAT | IPC_EXCL)) == -1)
		{
		if ((shmid = shmget(IPCKey, sizeof(struct IPCData)*IP_NUM, 0)) == -1)
			{
			printf("Error allocating %d bytes of IPC Shared memory, or attaching to existing segment. Do you have System V IPC turned on in your kernel? Do you have enough? Has a memory segment already been created with my ID that I don't have permisions to?\n", sizeof(struct IPCData)*IP_NUM);
			exit(0);
			}

		shmctl(shmid, IPC_STAT, &shmstatus);
		if (shmstatus.shm_nattch == 0)
			{
			shmctl(shmid, IPC_RMID, &shmstatus);
			if ((shmid = shmget(IPCKey, sizeof(struct IPCData)*IP_NUM, IPC_CREAT | IPC_EXCL)) == -1)
				{
				printf("Shared memory busted.  I am busted, everything is busted.  Have a nice day\n");
				exit(1);
				}
			}
		else
			{
#ifndef BSD
			printf("My shared memory segment %d is already in use (%ld locks), perhaps bandwidthd is already running in this directory?\n", shmid, shmstatus.shm_nattch);
#else
			printf("My shared memory segment %d is already in use (%hd locks), perhaps bandwidthd is already running in this directory?\n", shmid, shmstatus.shm_nattch);
#endif
			exit(1);
			}
		}
	
	if ((int) (IPCSharedData = shmat(shmid, 0, 0)) == -1)
		{
		printf("Error attaching to shared memory segment!\n");
		exit(0);
		}

    IntervalStart = time(NULL);

	printf("Opening %s\n", config.dev);	
	pd = pcap_open_live(config.dev, 100, config.promisc, 1000, Error);
        if (pd == NULL) printf("%s\n", Error);

    if (pcap_compile(pd, &fcode, config.filter, 1, 0) < 0)
		{
        pcap_perror(pd, "Error");
		printf("Exiting do to malformed filter string in bandwidthd.conf...\n");
		exit(1);
		}

    if (pcap_setfilter(pd, &fcode) < 0)
        pcap_perror(pd, "Error");

	switch (DataLink = pcap_datalink(pd))
		{
		default:
			if (config.dev)
				printf("Unknown Datalink Type %d, defaulting to ethernet\nPlease forward this error message and a packet sample (captured with \"tcpdump -i %s -s 2000 -n -w capture.cap\") to hinkle@derbyworks.net\n", DataLink, config.dev);
			else
				printf("Unknown Datalink Type %d, defaulting to ethernet\nPlease forward this error message and a packet sample (captured with \"tcpdump -s 2000 -n -w capture.cap\") to hinkle@derbyworks.net\n", DataLink);
		case DLT_EN10MB:
			printf("Packet Encoding:\n\tEthernet\n");
			IP_Offset = sizeof(struct ether_header);
			break;	
#ifdef DLT_LINUX_SLL 
		case DLT_LINUX_SLL:
			printf("Packet Encoding:\n\tLinux Cooked Socket\n");
			IP_Offset = 16;
			break;
#endif
#ifdef DLT_RAW
		case DLT_RAW:
			printf("Untested Datalink Type %d\nPlease report to hinkle@derbyworks.net if bandwidthd works for you\non this interface\n", DataLink);
			printf("Packet Encoding:\n\tRaw\n");
			IP_Offset = 0;
			break;
#endif
		case DLT_IEEE802:
			printf("Untested Datalink Type %d\nPlease report to hinkle@derbyworks.net if bandwidthd works for you\non this interface\n", DataLink);
			printf("Packet Encoding:\nToken Ring\n");
			IP_Offset = 22;
			break;
		}
                                           
	if (config.tag == '1') // We only rotate the first stage logs right now
		signal(SIGHUP, signal_handler);
	else
		signal(SIGHUP, SIG_IGN);

	if (IPDataStore)  // If there is data in the datastore draw some initial graphs
		{
		printf("Drawing initial graphs...\n");
		WriteOutWebpages(IntervalStart+config.interval);
		}

    if (pcap_loop(pd, -1, PacketCallback, pcap_userdata) < 0) {
        (void)fprintf(stderr, "Bandwidthd: pcap_loop: %s\n",  pcap_geterr(pd));
        exit(1);
        }

    pcap_close(pd);
    exit(0);        
    }
   
void PacketCallback(u_char *user, const struct pcap_pkthdr *h, const u_char *p)
    {
    unsigned int counter;

    u_int caplen = h->caplen;
    const struct ip *ip;

    uint32_t srcip;
    uint32_t dstip;


    struct IPData *ptrIPData;

    if (h->ts.tv_sec > IntervalStart + config.interval)  // Then write out this intervals data and possibly kick off the grapher
        {
        GraphIntervalCount++;
        CommitData(IntervalStart+config.interval);
		IpCount = 0;
        IntervalStart=h->ts.tv_sec;
        }

    caplen -= IP_Offset;  // We're only measuring ip size, so pull off the ethernet header
    p += IP_Offset; // Move the pointer past the datalink header

    ip = (const struct ip *)p; // Point ip at the ip header

	if (ip->ip_v != 4) // then not an ip packet so skip it
		return;

    srcip = ntohl(*(uint32_t *) (&ip->ip_src));
    dstip = ntohl(*(uint32_t *) (&ip->ip_dst));
	
    for (counter = 0; counter < SubnetCount; counter++)
        {	 
		// Packets from a monitored subnet to a monitored subnet will be
		// credited to both ip's

        if (SubnetTable[counter].ip == (srcip & SubnetTable[counter].mask))
            {
            ptrIPData = FindIp(srcip);  // Return or create this ip's data structure
			if (ptrIPData)
	            Credit(&(ptrIPData->Send), ip);

            ptrIPData = FindIp(0);  // Totals
			if (ptrIPData)
	            Credit(&(ptrIPData->Send), ip);
            }
    
        if (SubnetTable[counter].ip == (dstip & SubnetTable[counter].mask))
            {
            ptrIPData = FindIp(dstip);
    		if (ptrIPData)
		        Credit(&(ptrIPData->Receive), ip);

            ptrIPData = FindIp(0);
    		if (ptrIPData)
		        Credit(&(ptrIPData->Receive), ip);
            }                        
        }
    }

inline void Credit(struct Statistics *Stats, const struct ip *ip)
    {
    unsigned long size;
    const struct tcphdr *tcp;
    uint16_t sport, dport;

    size = ntohs(ip->ip_len);

    Stats->total += size;
    
    switch(ip->ip_p)
        {
        case 6:     // TCP
            tcp = (struct tcphdr *)(ip+1);
			tcp = (struct tcphdr *) ( ((char *)tcp) + ((ip->ip_hl-5)*4) ); // Compensate for IP Options
            Stats->tcp += size;
#if defined(SOLARIS) || defined (BSD)
            sport = ntohs(tcp->th_sport);
            dport = ntohs(tcp->th_dport);			
#else
            sport = ntohs(tcp->source);
            dport = ntohs(tcp->dest);
#endif
            if (sport == 80 || dport == 80 || sport == 443 || dport == 443)
                Stats->http += size;
	
			if (sport == 20 || dport == 20 || sport == 21 || dport == 21)
				Stats->ftp += size;

            if (sport == 1044|| dport == 1044||		// Direct File Express
				sport == 1045|| dport == 1045|| 	// ''  <- Dito Marks
                sport == 1214|| dport == 1214||		// Grokster, Kaza, Morpheus
				sport == 4661|| dport == 4661||		// EDonkey 2000
				sport == 4662|| dport == 4662||     // ''
				sport == 4665|| dport == 4665||     // ''
				sport == 5190|| dport == 5190||		// Song Spy
				sport == 5500|| dport == 5500||		// Hotline Connect
				sport == 5501|| dport == 5501||		// ''
				sport == 5502|| dport == 5502||		// ''
				sport == 5503|| dport == 5503||		// ''
				sport == 6346|| dport == 6346||		// Gnutella Engine
				sport == 6347|| dport == 6347||		// ''
				sport == 6666|| dport == 6666||		// Yoink
				sport == 6667|| dport == 6667||		// ''
				sport == 7788|| dport == 7788||		// Budy Share
				sport == 8888|| dport == 8888||		// AudioGnome, OpenNap, Swaptor
				sport == 8889|| dport == 8889||		// AudioGnome, OpenNap
				sport == 28864|| dport == 28864||	// hotComm				
				sport == 28865|| dport == 28865)	// hotComm
                Stats->p2p += size;
            break;
        case 17:
            Stats->udp += size;
            break;
        case 1: 
            Stats->icmp += size;
            break;
        }
    }

// TODO:  Throw away old data!
void DropOldData(long int timestamp) 	// Go through the ram datastore and dump old data
	{
	struct IPDataStore *DataStore;
	struct IPDataStore *PrevDataStore;	
	struct DataStoreBlock *DeletedBlock;
	
	PrevDataStore = NULL;
    DataStore = IPDataStore;

	// Progress through the linked list until we reach the end
	while(DataStore)  // we have data
		{
		// If the First block is out of date, purge it, if it is the only block
		// purge the node
        while(DataStore->FirstBlock->LatestTimestamp < timestamp - config.range)
			{
            if ((!DataStore->FirstBlock->Next) && PrevDataStore) // There is no valid block of data for this ip, so unlink the whole ip
				{ 												// Don't bother unlinking the ip if it's the first one, that's to much
																// Trouble
				PrevDataStore->Next = DataStore->Next;	// Unlink the node
				free(DataStore->FirstBlock->Data);      // Free the memory
				free(DataStore->FirstBlock);
				free(DataStore);												
				DataStore = PrevDataStore->Next;	// Go to the next node
				if (!DataStore) return; // We're done
				}				
			else if (!DataStore->FirstBlock->Next)
				{
				// There is no valid block of data for this ip, and we are 
				// the first ip, so do nothing 
				break; // break out of this loop so the outside loop increments us
				} 
			else // Just unlink this block
				{
				DeletedBlock = DataStore->FirstBlock;
				DataStore->FirstBlock = DataStore->FirstBlock->Next;	// Unlink the block
				free(DeletedBlock->Data);
				free(DeletedBlock);
			    }
			}

		PrevDataStore = DataStore;				
		DataStore = DataStore->Next;
		}
	}

void StoreIPDataInCDF(struct IPData IncData[])
	{
	struct IPData *IPData;
	unsigned int counter;
	FILE *cdf;
	struct Statistics *Stats;
	char IPBuffer[50];
	char logfile[] = "log1.cdf";
	

	if (config.tag != '1')
		{ 
		logfile[3] = config.tag;
    	cdf = fopen(logfile, "a");
		}
	else
		cdf = fopen("log.cdf", "a");

	for (counter=0; counter < IpCount; counter++)
		{
		IPData = &IncData[counter];
		HostIp2CharIp(IPData->ip, IPBuffer);
		fprintf(cdf, "%s,%lu,", IPBuffer, IPData->timestamp);
		Stats = &(IPData->Send);
		fprintf(cdf, "%llu,%llu,%llu,%llu,%llu,%llu,%llu,", Stats->total, Stats->icmp, Stats->udp, Stats->tcp, Stats->ftp, Stats->http, Stats->p2p); 
		Stats = &(IPData->Receive);
		fprintf(cdf, "%llu,%llu,%llu,%llu,%llu,%llu,%llu\n", Stats->total, Stats->icmp, Stats->udp, Stats->tcp, Stats->ftp, Stats->http, Stats->p2p); 		
		}
	fclose(cdf);
	}



void _StoreIPDataInRam(struct IPData *IPData)
	{
	struct IPDataStore *DataStore;
	struct DataStoreBlock *DataStoreBlock;

	if (!IPDataStore) // we need to create the first entry
		{
		// Allocate Datastore for this IP
	    IPDataStore = malloc(sizeof(struct IPDataStore));
			
		IPDataStore->ip = IPData->ip;
		IPDataStore->Next = NULL;
					
		// Allocate it's first block of storage
		IPDataStore->FirstBlock = malloc(sizeof(struct DataStoreBlock));
		IPDataStore->FirstBlock->LatestTimestamp = 0;

		IPDataStore->FirstBlock->NumEntries = 0;
		IPDataStore->FirstBlock->Data = calloc(IPDATAALLOCCHUNKS, sizeof(struct IPData));
		IPDataStore->FirstBlock->Next = NULL;																		
        if (!IPDataStore->FirstBlock || ! IPDataStore->FirstBlock->Data)
            {
            printf("Could not allocate datastore! Exiting!\n");
            exit(1);
            }
		}

	DataStore = IPDataStore;

	// Take care of first case
	while (DataStore) // Is not null
		{
		if (DataStore->ip == IPData->ip) // then we have the right store
			{
			DataStoreBlock = DataStore->FirstBlock;

			while(DataStoreBlock) // is not null
				{
				if (DataStoreBlock->NumEntries < IPDATAALLOCCHUNKS) // We have a free spot
					{
					memcpy(&DataStoreBlock->Data[DataStoreBlock->NumEntries++], IPData, sizeof(struct IPData));
					DataStoreBlock->LatestTimestamp = IPData->timestamp;
					return;
					}
        	    else
					{
					if (!DataStoreBlock->Next) // there isn't another block, add one
						{
	                    DataStoreBlock->Next = malloc(sizeof(struct DataStoreBlock));
						DataStoreBlock->Next->LatestTimestamp = 0;
						DataStoreBlock->Next->NumEntries = 0;
						DataStoreBlock->Next->Data = calloc(IPDATAALLOCCHUNKS, sizeof(struct IPData));
						DataStoreBlock->Next->Next = NULL;																				
						}

					DataStoreBlock = DataStoreBlock->Next;
					}
				}						

			return;
			}
		else
			{
			if (!DataStore->Next) // there is no entry for this ip, so lets make one.
				{
				// Allocate Datastore for this IP
    	        DataStore->Next = malloc(sizeof(struct IPDataStore));
				
			    DataStore->Next->ip = IPData->ip;
				DataStore->Next->Next = NULL;
					
				// Allocate it's first block of storage
				DataStore->Next->FirstBlock = malloc(sizeof(struct DataStoreBlock));
				DataStore->Next->FirstBlock->LatestTimestamp = 0;
				DataStore->Next->FirstBlock->NumEntries = 0;
				DataStore->Next->FirstBlock->Data = calloc(IPDATAALLOCCHUNKS, sizeof(struct IPData));
				DataStore->Next->FirstBlock->Next = NULL;																		
				}
	
			DataStore = DataStore->Next;			
			}
		}
	}

void StoreIPDataInRam(struct IPData IncData[])
	{
	unsigned int counter;

    for (counter=0; counter < IpCount; counter++)
		_StoreIPDataInRam(&IncData[counter]);
	}


void CommitData(long int timestamp)
    {
	static int MayGraph = TRUE;
    unsigned int counter;
	struct stat StatBuf;

	// Set the timestamps
	for (counter=0; counter < IpCount; counter++)
        IpTable[counter].timestamp = timestamp;

	// Output modules
	StoreIPDataInRam(IpTable);
	// TODO: This needs to be moved into the forked section, but I don't want to 
	//	deal with that right now (Heavy disk io may make us drop packets)
	if (config.output_cdf) StoreIPDataInCDF(IpTable);

	if (RotateLogs) // We set this to true on HUP
		{
		if (!stat("log.5.cdf", &StatBuf)) // File exists
			unlink("log.5.cdf");
		if (!stat("log.4.cdf", &StatBuf)) // File exists
			rename("log.4.cdf", "log.5.cdf");
		if (!stat("log.3.cdf", &StatBuf)) // File exists
			rename("log.3.cdf", "log.4.cdf");
		if (!stat("log.2.cdf", &StatBuf)) // File exists
			rename("log.2.cdf", "log.3.cdf");
		if (!stat("log.1.cdf", &StatBuf)) // File exists
			rename("log.1.cdf", "log.2.cdf");
		if (!stat("log.cdf", &StatBuf)) // File exists
			rename("log.cdf", "log.1.cdf");					
		fclose(fopen("log.cdf", "a")); // Touch file
		RotateLogs = FALSE;
		}

	// Reap a couple zombies
	if (waitpid(-1, NULL, WNOHANG))  // A child was reaped
		MayGraph = TRUE;

	if (GraphIntervalCount%config.skip_intervals == 0 && MayGraph)
		{
		MayGraph = FALSE;
		WriteOutWebpages(timestamp);
		}
	else if (GraphIntervalCount%config.skip_intervals == 0)
		printf("Previouse graphing run not complete... Skipping current run\n");

	DropOldData(timestamp);
    }


int RCDF_Test(char *filename)
	{
	// Determine if the first date in the file is before the cutoff
	// return FALSE on error
	FILE *cdf;
	char ipaddrBuffer[16];
	time_t timestamp;

	if (!(cdf = fopen(filename, "r"))) return FALSE;
	if(fscanf(cdf, " %15[0-9.],%lu,", ipaddrBuffer, &timestamp) != 2) return FALSE;
	if (timestamp > time(NULL) - config.range)
		return FALSE; // Keep looking
	else
		return TRUE; // Start with this file
	}


void RCDF_PositionStream(FILE *cdf)
	{
    time_t timestamp;
	time_t current_timestamp;
	char ipaddrBuffer[16];

	current_timestamp = time(NULL);

	fseek(cdf, 0, SEEK_END);
	timestamp = current_timestamp;
	while (timestamp > current_timestamp - config.range)
		{
		// What happenes if we seek past the beginning of the file?
		if (fseek(cdf, -IP_NUM*75*(config.range/config.interval)/20,SEEK_CUR))
			{ // fseek returned error, just seek to beginning
			printf("Seeked past beginning of file, loading from beginning...\n");
			fseek(cdf, 0, SEEK_SET);
			return;
			}
		while (fgetc(cdf) != '\n' && !feof(cdf)); // Read to next line
		ungetc('\n', cdf);  // Just so the fscanf mask stays identical
        if(fscanf(cdf, " %15[0-9.],%lu,", ipaddrBuffer, &timestamp) != 2)
			{
			printf("Unknown error while scanning for beginning of data...\n");
			return;	
			}
		}
	while (fgetc(cdf) != '\n' && !feof(cdf));
	ungetc('\n', cdf); 
	}

void RCDF_Load(FILE *cdf)
	{
    time_t timestamp;
	time_t current_timestamp = 0;
	struct in_addr ipaddr;
	struct IPData *ip=NULL;
	char ipaddrBuffer[16];
	unsigned long int Counter = 0;
	unsigned long int IntervalsRead = 0;

    for(Counter = 0; !feof(cdf) && !ferror(cdf); Counter++)
	    {
		if(fscanf(cdf, " %15[0-9.],%lu,", ipaddrBuffer, &timestamp) != 2) 
			goto End_RecoverDataFromCdf;

		if (!timestamp) // First run through loop
			current_timestamp = timestamp;

		if (timestamp != current_timestamp)
			{ // Dump to datastore
			StoreIPDataInRam(IpTable);
			IpCount = 0; // Reset Traffic Counters
			current_timestamp = timestamp;
			IntervalsRead++;
			}    		
		inet_aton(ipaddrBuffer, &ipaddr);
		ip = FindIp(ntohl(ipaddr.s_addr));
		ip->timestamp = timestamp;

        if (fscanf(cdf, "%llu,%llu,%llu,%llu,%llu,%llu,%llu,",
            &ip->Send.total, &ip->Send.icmp, &ip->Send.udp,
            &ip->Send.tcp, &ip->Send.ftp, &ip->Send.http, &ip->Send.p2p) != 7
          || fscanf(cdf, "%llu,%llu,%llu,%llu,%llu,%llu,%llu",
            &ip->Receive.total, &ip->Receive.icmp, &ip->Receive.udp,
            &ip->Receive.tcp, &ip->Receive.ftp, &ip->Receive.http, &ip->Receive.p2p) != 7)
			goto End_RecoverDataFromCdf;		

		if (((Counter%30000) == 0) && Counter > 0)
			printf("%lu records read over %lu intervals...\n", Counter, IntervalsRead);
		}

End_RecoverDataFromCdf:
	StoreIPDataInRam(IpTable);
	printf("%lu records total...\n", Counter);	
	DropOldData(time(NULL)); // Dump the extra data
    if(!feof(cdf))
       printf("Failed to parse part of log file. Giving up on the file.\n");
	IpCount = 0; // Reset traffic counters
    fclose(cdf);
	}

void RecoverDataFromCDF(void)
	{
	FILE *cdf;
    struct stat StatBuf;
	char filename[] = "log2.cdf";

	if (config.tag != '1') // We don't rotate weekly and monthly logs right now
		{
		filename[3] = config.tag;
		if (stat(filename, &StatBuf))
			return;
		
        // Just recover the log2.cdf and return
        if (!(cdf = fopen(filename, "r"))) return;
        printf("Recovering from %s...\n", filename);
        RCDF_PositionStream(cdf);
        RCDF_Load(cdf);
        return;						
		}

	if (stat("log.cdf", &StatBuf))
		return;	

	if (RCDF_Test("log.cdf") || stat("log.1.cdf", &StatBuf))  
		{
		// Simple case, just recover the log.cdf and return
		if (!(cdf = fopen("log.cdf", "r"))) return;
		printf("Recovering from log.cdf...\n");
		RCDF_PositionStream(cdf);
		RCDF_Load(cdf);
		return;
		}	
	else if (RCDF_Test("log.1.cdf") || stat("log.2.cdf", &StatBuf))
		{
		if (!(cdf = fopen("log.1.cdf", "r"))) return;
		printf("Recovering from log.1.cdf...\n");
		RCDF_PositionStream(cdf);
		RCDF_Load(cdf);
		if (!(cdf = fopen("log.cdf", "r"))) return;		
		printf("Recovering from log.cdf...\n");
		RCDF_Load(cdf);
		}
	else if (RCDF_Test("log.2.cdf") || stat("log.3.cdf", &StatBuf))
		{
		if (!(cdf = fopen("log.2.cdf", "r"))) return;
		printf("Recovering from log.2.cdf...\n");
		RCDF_PositionStream(cdf);
		RCDF_Load(cdf);
		if (!(cdf = fopen("log.1.cdf", "r"))) return;		
		printf("Recovering from log.1.cdf...\n");
		RCDF_Load(cdf);
		if (!(cdf = fopen("log.cdf", "r"))) return;		
		printf("Recovering from log.cdf...\n");
		RCDF_Load(cdf);
		}
	else if (RCDF_Test("log.3.cdf") || stat("log.4.cdf", &StatBuf))
		{
		if (!(cdf = fopen("log.3.cdf", "r"))) return;
		printf("Recovering from log.3.cdf...\n");
		RCDF_PositionStream(cdf);
		RCDF_Load(cdf);
		if (!(cdf = fopen("log.2.cdf", "r"))) return;		
		printf("Recovering from log.2.cdf...\n");
		RCDF_Load(cdf);
		if (!(cdf = fopen("log.1.cdf", "r"))) return;		
		printf("Recovering from log.1.cdf...\n");
		RCDF_Load(cdf);
		if (!(cdf = fopen("log.cdf", "r"))) return;		
		printf("Recovering from log.cdf...\n");
		RCDF_Load(cdf);
		}
	else if (RCDF_Test("log.4.cdf") || stat("log.5.cdf", &StatBuf))
		{
		if (!(cdf = fopen("log.4.cdf", "r"))) return;
		printf("Recovering from log.4.cdf...\n");
		RCDF_PositionStream(cdf);
		RCDF_Load(cdf);
		if (!(cdf = fopen("log.3.cdf", "r"))) return;		
		printf("Recovering from log.3.cdf...\n");
		RCDF_Load(cdf);
		if (!(cdf = fopen("log.2.cdf", "r"))) return;		
		printf("Recovering from log.2.cdf...\n");
		RCDF_Load(cdf);
		if (!(cdf = fopen("log.1.cdf", "r"))) return;		
		printf("Recovering from log.1.cdf...\n");
		RCDF_Load(cdf);
		if (!(cdf = fopen("log.cdf", "r"))) return;		
		printf("Recovering from log.cdf...\n");
		RCDF_Load(cdf);
		}
	else
		{
		if (!(cdf = fopen("log.5.cdf", "r"))) return;
		printf("Recovering from log.5.cdf...\n");
		RCDF_PositionStream(cdf);
		RCDF_Load(cdf);
		if (!(cdf = fopen("log.4.cdf", "r"))) return;		
		printf("Recovering from log.4.cdf...\n");
		RCDF_Load(cdf);
		if (!(cdf = fopen("log.3.cdf", "r"))) return;		
		printf("Recovering from log.3.cdf...\n");
		RCDF_Load(cdf);
		if (!(cdf = fopen("log.2.cdf", "r"))) return;		
		printf("Recovering from log.2.cdf...\n");
		RCDF_Load(cdf);
		if (!(cdf = fopen("log.1.cdf", "r"))) return;		
		printf("Recovering from log.1.cdf...\n");
		RCDF_Load(cdf);
		if (!(cdf = fopen("log.cdf", "r"))) return;		
		printf("Recovering from log.cdf...\n");
		RCDF_Load(cdf);
		}
	}

// ****** FindIp **********
// ****** Returns or allocates an Ip's data structure

inline struct IPData *FindIp(uint32_t ipaddr)
    {
    unsigned int counter;
    
    for (counter=0; counter < IpCount; counter++)
        if (IpTable[counter].ip == ipaddr)
            return (&IpTable[counter]);
    
    if (IpCount >= IP_NUM)
        {
        printf("IP_NUM is too low, dropping ip....");
       	return(NULL);
        }
	
    memset(&IpTable[IpCount], 0, sizeof(struct IPData));
    IpTable[IpCount].Magick[0] = 'R';
    IpTable[IpCount].Magick[1] = 'E';
    IpTable[IpCount].Magick[2] = 'C';
    IpTable[IpCount].Magick[3] = 'B';

    IpTable[IpCount].ip = ipaddr;
    return (&IpTable[IpCount++]);
    }

size_t ICGrandTotalDataPoints = 0;

char inline *HostIp2CharIp(unsigned long ipaddr, char *buffer)
    {
	struct in_addr in_addr;
	char *s;

	in_addr.s_addr = htonl(ipaddr);	
    s = inet_ntoa(in_addr);
	strncpy(buffer, s, 16);
	buffer[15] = '\0';
	return(buffer);
/*  uint32_t ip = *(uint32_t *)ipaddr;

	sprintf(buffer, "%d.%d.%d.%d", (ip << 24)  >> 24, (ip << 16) >> 24, (ip << 8) >> 24, (ip << 0) >> 24);
*/
    }

// Add better error checking

int fork2()
    {
    pid_t pid;

    if (!(pid = fork()))
        {
        if (!fork())
        	{
#ifdef PROFILE
				// Got this incantation from a message board.  Don't forget to set
				// GMON_OUT_PREFIX in the shell
				extern void _start(void), etext(void);
				printf("Calling profiler startup...\n");
				monstartup((u_long) &_start, (u_long) &etext);
#endif
            return(0);
            }        

        _exit(0);
        }
    
    waitpid(pid, NULL, 0);
    return(1);
    }

