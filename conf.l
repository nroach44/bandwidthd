%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "conf.tab.h"

%}

%%
[ \t]+      			/* Ignore */;
\n						{ return TOKNEWLINE; }
#.*\n        			{ return TOKNEWLINE; }
([0-9]{1,3}"."){3,3}([0-9]{1,3}) { yylval.string=strdup(yytext); return IPADDR; }
[-0-9]+					{ yylval.number=atoi(yytext); return NUMBER; }
\".*\"					{ yylval.string=strdup(yytext); return STRING; }
\/						{ return TOKSLASH; }
true|false              { yylval.number=!strcmp(yytext,"true"); return STATE; }
subnet					{ return TOKSUBNET; }
dev						{ return TOKDEV; }
skip_intervals			{ return TOKSKIPINTERVALS; }
graph_cutoff			{ return TOKGRAPHCUTOFF; }
promiscuous				{ return TOKPROMISC; }
output_cdf				{ return TOKOUTPUTCDF; }		
recover_cdf				{ return TOKRECOVERCDF; }		
graph					{ return TOKGRAPH; }
.						{ return TOKJUNK; }
%%
