typedef union
{
    int number;
    char *string;
} YYSTYPE;
#define	TOKJUNK	257
#define	TOKSUBNET	258
#define	TOKDEV	259
#define	TOKSLASH	260
#define	TOKSKIPINTERVALS	261
#define	TOKGRAPHCUTOFF	262
#define	TOKPROMISC	263
#define	TOKOUTPUTCDF	264
#define	TOKRECOVERCDF	265
#define	TOKGRAPH	266
#define	IPADDR	267
#define	NUMBER	268
#define	STRING	269
#define	STATE	270


extern YYSTYPE yylval;
