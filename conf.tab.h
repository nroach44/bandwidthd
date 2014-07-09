/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_BDCONFIG_Y_TAB_H_INCLUDED
# define YY_BDCONFIG_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int bdconfig_debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    TOKJUNK = 258,
    TOKSUBNET = 259,
    TOKDEV = 260,
    TOKSLASH = 261,
    TOKSKIPINTERVALS = 262,
    TOKGRAPHCUTOFF = 263,
    TOKDESCRIPTION = 264,
    TOKPROMISC = 265,
    TOKOUTPUTCDF = 266,
    TOKRECOVERCDF = 267,
    TOKGRAPH = 268,
    TOKNEWLINE = 269,
    TOKFILTER = 270,
    TOKMANAGEMENTURL = 271,
    TOKMETAREFRESH = 272,
    TOKPGSQLCONNECTSTRING = 273,
    TOKSENSORID = 274,
    TOKHTDOCSDIR = 275,
    TOKLOGDIR = 276,
    TOKEXTENSIONS = 277,
    TOKSQLITEFILENAME = 278,
    IPADDR = 279,
    NUMBER = 280,
    STRING = 281,
    STATE = 282
  };
#endif
/* Tokens.  */
#define TOKJUNK 258
#define TOKSUBNET 259
#define TOKDEV 260
#define TOKSLASH 261
#define TOKSKIPINTERVALS 262
#define TOKGRAPHCUTOFF 263
#define TOKDESCRIPTION 264
#define TOKPROMISC 265
#define TOKOUTPUTCDF 266
#define TOKRECOVERCDF 267
#define TOKGRAPH 268
#define TOKNEWLINE 269
#define TOKFILTER 270
#define TOKMANAGEMENTURL 271
#define TOKMETAREFRESH 272
#define TOKPGSQLCONNECTSTRING 273
#define TOKSENSORID 274
#define TOKHTDOCSDIR 275
#define TOKLOGDIR 276
#define TOKEXTENSIONS 277
#define TOKSQLITEFILENAME 278
#define IPADDR 279
#define NUMBER 280
#define STRING 281
#define STATE 282

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 39 "conf.y" /* yacc.c:1909  */

    int number;
    char *string;

#line 113 "y.tab.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE bdconfig_lval;

int bdconfig_parse (void);

#endif /* !YY_BDCONFIG_Y_TAB_H_INCLUDED  */
