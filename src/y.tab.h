/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     STATEMENT = 258,
     LIST = 259,
     LIST_ELEMENT = 260,
     VAR = 261,
     OBJ = 262,
     ARR = 263,
     ALLDIFF = 264,
     LE_CON = 265,
     GE_CON = 266,
     EQ_CON = 267,
     NE_CON = 268,
     IMPLY_CON = 269,
     UNTIL_CON = 270,
     LT_OP = 271,
     GT_OP = 272,
     LE_OP = 273,
     GE_OP = 274,
     EQ_OP = 275,
     NE_OP = 276,
     MAX_OP = 277,
     MIN_OP = 278,
     AND_OP = 279,
     OR_OP = 280,
     NOT_OP = 281,
     AT = 282,
     FIRST = 283,
     NEXT = 284,
     FBY = 285,
     IF = 286,
     THEN = 287,
     ELSE = 288,
     ABS = 289,
     IDENTIFIER = 290,
     ARR_IDENTIFIER = 291,
     CONSTANT = 292
   };
#endif
/* Tokens.  */
#define STATEMENT 258
#define LIST 259
#define LIST_ELEMENT 260
#define VAR 261
#define OBJ 262
#define ARR 263
#define ALLDIFF 264
#define LE_CON 265
#define GE_CON 266
#define EQ_CON 267
#define NE_CON 268
#define IMPLY_CON 269
#define UNTIL_CON 270
#define LT_OP 271
#define GT_OP 272
#define LE_OP 273
#define GE_OP 274
#define EQ_OP 275
#define NE_OP 276
#define MAX_OP 277
#define MIN_OP 278
#define AND_OP 279
#define OR_OP 280
#define NOT_OP 281
#define AT 282
#define FIRST 283
#define NEXT 284
#define FBY 285
#define IF 286
#define THEN 287
#define ELSE 288
#define ABS 289
#define IDENTIFIER 290
#define ARR_IDENTIFIER 291
#define CONSTANT 292




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 34 "stcsp.y"
{
    char *str;
    int num;
    Node *node;
}
/* Line 1529 of yacc.c.  */
#line 129 "y.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

