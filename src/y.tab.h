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
     LE_CON = 264,
     GE_CON = 265,
     EQ_CON = 266,
     NE_CON = 267,
     IMPLY_CON = 268,
     UNTIL_CON = 269,
     LT_OP = 270,
     GT_OP = 271,
     LE_OP = 272,
     GE_OP = 273,
     EQ_OP = 274,
     NE_OP = 275,
     MAX_OP = 276,
     MIN_OP = 277,
     AND_OP = 278,
     OR_OP = 279,
     NOT_OP = 280,
     AT = 281,
     FIRST = 282,
     NEXT = 283,
     FBY = 284,
     IF = 285,
     THEN = 286,
     ELSE = 287,
     ABS = 288,
     IDENTIFIER = 289,
     ARR_IDENTIFIER = 290,
     CONSTANT = 291
   };
#endif
/* Tokens.  */
#define STATEMENT 258
#define LIST 259
#define LIST_ELEMENT 260
#define VAR 261
#define OBJ 262
#define ARR 263
#define LE_CON 264
#define GE_CON 265
#define EQ_CON 266
#define NE_CON 267
#define IMPLY_CON 268
#define UNTIL_CON 269
#define LT_OP 270
#define GT_OP 271
#define LE_OP 272
#define GE_OP 273
#define EQ_OP 274
#define NE_OP 275
#define MAX_OP 276
#define MIN_OP 277
#define AND_OP 278
#define OR_OP 279
#define NOT_OP 280
#define AT 281
#define FIRST 282
#define NEXT 283
#define FBY 284
#define IF 285
#define THEN 286
#define ELSE 287
#define ABS 288
#define IDENTIFIER 289
#define ARR_IDENTIFIER 290
#define CONSTANT 291




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 33 "stcsp.y"
{
    char *str;
    int num;
    Node *node;
}
/* Line 1529 of yacc.c.  */
#line 127 "y.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

