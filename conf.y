%{
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#ifdef BSD
#include <sys/wait.h>
#else
#include <wait.h>
#include <malloc.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "bandwidthd.h"

extern unsigned int SubnetCount;
extern struct SubnetData SubnetTable[];
extern struct config config;

int yylex(void);
int LineNo = 1;

void yyerror(const char *str)
    {
    fprintf(stderr, "Syntax Error \"%s\" on line %d\n", str, LineNo);
	exit(1);
    }

int yywrap()
	{
	return(1);
	}
%}

%token TOKJUNK TOKSUBNET TOKDEV TOKSLASH TOKSKIPINTERVALS TOKGRAPHCUTOFF 
%token TOKPROMISC TOKOUTPUTCDF TOKRECOVERCDF TOKGRAPH TOKNEWLINE TOKFILTER
%token TOKMETAREFRESH
%union
{
    int number;
    char *string;
}

%token <string> IPADDR
%token <number> NUMBER
%token <string> STRING
%token <number> STATE
%type <string> string
%%

commands: /* EMPTY */
    | commands command
    ;

command:
	subnet
	|
	device
	|
	skip_intervals
	|
	graph_cutoff
	|
	promisc
	|
	output_cdf
	|
	recover_cdf
	|
	graph
	|
	newline
	|
	filter
	|
	meta_refresh
	;

subnet:
	subneta
	|
	subnetb
	;

newline:
	TOKNEWLINE
	{
	LineNo++;
	}
	;

subneta:
	TOKSUBNET IPADDR IPADDR
	{
	struct in_addr addr;

	SubnetTable[SubnetCount].ip = inet_network($2) & inet_network($3);
    	SubnetTable[SubnetCount].mask = inet_network($3);	

	addr.s_addr = ntohl(SubnetTable[SubnetCount].ip);
	printf("Monitoring subnet %s ", inet_ntoa(addr));
	addr.s_addr = ntohl(SubnetTable[SubnetCount++].mask);
	printf("with netmask %s\n", inet_ntoa(addr));
	}
	;

subnetb:
	TOKSUBNET IPADDR TOKSLASH NUMBER
	{
	unsigned int Subnet, Counter, Mask;
	struct in_addr addr;

	Mask = 1; Mask <<= 31;
	for (Counter = 0, Subnet = 0; Counter < $4; Counter++)
		{
		Subnet >>= 1;
		Subnet |= Mask;
		}
 	SubnetTable[SubnetCount].mask = Subnet; 
	SubnetTable[SubnetCount].ip = inet_network($2) & Subnet;
	addr.s_addr = ntohl(SubnetTable[SubnetCount].ip);
	printf("Monitoring subnet %s ", inet_ntoa(addr));
	addr.s_addr = ntohl(SubnetTable[SubnetCount++].mask);
	printf("with netmask %s\n", inet_ntoa(addr));
	}
	;

string:
    STRING
    {
    $1[strlen($1)-1] = '\0';
    $$ = $1+1;
    }
    ;

device:
	TOKDEV string
	{
	config.dev = $2;
	}

filter:
	TOKFILTER string
	{
	config.filter = $2;
	}

meta_refresh:
	TOKMETAREFRESH NUMBER
	{
	config.meta_refresh = $2;
	}

skip_intervals:
	TOKSKIPINTERVALS NUMBER
	{
	config.skip_intervals = $2+1;
	}

graph_cutoff:
	TOKGRAPHCUTOFF NUMBER
	{
	config.graph_cutoff = $2*1024;
	}

promisc:
	TOKPROMISC STATE
	{
	config.promisc = $2;
	}

output_cdf:
	TOKOUTPUTCDF STATE
	{
	config.output_cdf = $2;
	}

recover_cdf:
	TOKRECOVERCDF STATE
	{
	config.recover_cdf = $2;
	}

graph:
	TOKGRAPH STATE
	{
	config.graph = $2;
	}
