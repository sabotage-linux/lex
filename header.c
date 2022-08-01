/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2005 Sun Microsystems, Inc.
 * All rights reserved.
 * Use is subject to license terms.
 */

/*	Copyright (c) 1988 AT&T	*/
/*	All Rights Reserved	*/

/*	from OpenSolaris "header.c	6.22	05/06/08 SMI"	*/

/*
 * Portions Copyright (c) 2005 Gunnar Ritter, Freiburg i. Br., Germany
 *
 * Sccsid @(#)header.c	1.12 (gritter) 9/23/06
 */

#include "ldefs.h"

static void rhd1(void);
static void chd1(void);
static void chd2(void);
static void ctail(void);
static void rtail(void);

void
phead1(void)
{
#ifdef WITH_RATFOR
	ratfor ? rhd1() : chd1();
#else
	chd1();
#endif
}

static void
chd1(void)
{
	if (*v_stmp == 'y') {
		extern const char	rel[];
		fprintf(fout, "\
#if __GNUC__ >= 3 && __GNUC_MINOR__ >= 4 || __GNUC__ >= 4\n\
#define	YYUSED	__attribute__ ((used))\n\
#elif defined __GNUC__\n\
#define	YYUSED	__attribute__ ((unused))\n\
#else\n\
#define	YYUSED\n\
#endif\n\
static const char yylexid[] USED = \"lex: %s\"\n", rel);
	}
	if (handleeuc) {
		fprintf(fout,
			"#ifndef EUC\n"
			"#define EUC\n"
			"#endif\n"
			"#include <stdio.h>\n"
			"#include <stdlib.h>\n"
			"#ifdef	__sun\n"
			"#include <widec.h>\n"
			"#else	/* !__sun */\n"
			"#include <wchar.h>\n"
			"#endif	/* !__sun */\n");
		if (widecio) { /* -w option */
			fprintf(fout,
				"#define YYTEXT yytext\n"
				"#define YYLENG yyleng\n"
				"#ifndef __cplusplus\n"
				"#define YYINPUT input\n"
				"#define YYOUTPUT output\n"
				"#else\n"
				"#define YYINPUT lex_input\n"
				"#define YYOUTPUT lex_output\n"
				"#endif\n"
				"#define YYUNPUT unput\n");
		} else { /* -e option */
			fprintf(fout,
				"#include <limits.h>\n"
				"#ifdef	__sun\n"
				"#include <sys/euc.h>\n"
				"#endif	/* __sun */\n"
				"#define YYLEX_E 1\n"
				"#define YYTEXT yywtext\n"
				"#define YYLENG yywleng\n"
				"#define YYINPUT yywinput\n"
				"#define YYOUTPUT yywoutput\n"
				"#define YYUNPUT yywunput\n");
		}
	} else { /* ASCII compatibility mode. */
		fprintf(fout,
			"#include <stdio.h>\n"
			"#include <stdlib.h>\n");
	}
	if (ZCH > NCH)
		fprintf(fout, "# define U(x) ((x)&0377)\n");
	else
		fprintf(fout, "# define U(x) x\n");

	fprintf(fout,
		"# define NLSTATE yyprevious=YYNEWLINE\n"
		"# define BEGIN yybgin = yysvec + 1 +\n"
		"# define INITIAL 0\n"
		"# define YYLERR yysvec\n"
		"# define YYSTATE (yyestate-yysvec-1)\n");
	if (optim)
		fprintf(fout, "# define YYOPTIM 1\n");
#ifdef DEBUG
	fprintf(fout, "# define LEXDEBUG 1\n");
#endif
	fprintf(fout,
		"# ifndef YYLMAX \n"
		"# define YYLMAX BUFSIZ\n"
		"# endif \n"
		"#ifndef __cplusplus\n");
	if (widecio)
		fprintf(fout, "# define output(c) (void)putwc(c,yyout)\n");
	else
		fprintf(fout, "# define output(c) (void)putc(c,yyout)\n");
	fprintf(fout, "#else\n");
	if (widecio)
		fprintf(fout, "# define lex_output(c) (void)putwc(c,yyout)\n");
	else
		fprintf(fout, "# define lex_output(c) (void)putc(c,yyout)\n");
	fprintf(fout,
		"#endif\n"
		"\n#if defined(__cplusplus) || defined(__STDC__)\n"
		"\n#if defined(__cplusplus) && defined(__EXTERN_C__)\n"
		"extern \"C\" {\n"
		"#endif\n"
		"\tint yyback(int *, int);\n" /* ? */
		"\tint yyinput(void);\n" /* ? */
		"\tint yylook(void);\n" /* ? */
		"\tvoid yyoutput(int);\n" /* ? */
		"\tint yyracc(int);\n" /* ? */
		"\tint yyreject(void);\n" /* ? */
		"\tvoid yyunput(int);\n" /* ? */
		"\tint yylex(void);\n"
		"#ifdef YYLEX_E\n"
		"\tvoid yywoutput(wchar_t);\n"
		"\twchar_t yywinput(void);\n"
		"\tvoid yywunput(wchar_t);\n"
		"#endif\n"
	/* XCU4: type of yyless is int */
		"#ifndef yyless\n"
		"\tint yyless(int);\n"
		"#endif\n"
		"#ifndef yywrap\n"
		"\tint yywrap(void);\n"
		"#endif\n"
		"#ifdef LEXDEBUG\n"
		"\tvoid allprint(char);\n"
		"\tvoid sprint(char *);\n"
		"#endif\n"
		"#if defined(__cplusplus) && defined(__EXTERN_C__)\n"
		"}\n"
		"#endif\n\n"
		"#ifdef __cplusplus\n"
		"extern \"C\" {\n"
		"#endif\n"
		"\tvoid exit(int);\n"
		"#ifdef __cplusplus\n"
		"}\n"
		"#endif\n\n"
		"#endif\n"
		"# define unput(c)"
		" {yytchar= (c);if(yytchar=='\\n')yylineno--;*yysptr++=yytchar;}\n"
		"# define yymore() (yymorfg=1)\n");
	if (widecio) {
		fprintf(fout,
			"#ifndef __cplusplus\n"
			"%s%d%s\n",
"# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):yygetwchar())==",
		ctable['\n'],
"?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)");
		fprintf(fout,
			"#else\n"
			"%s%d%s\n",
"# define lex_input() (((yytchar=yysptr>yysbuf?U(*--yysptr):yygetwchar())==",
		ctable['\n'],
"?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)");
		fprintf(fout,
			"#endif\n"
			"# define ECHO (void)fprintf(yyout, \"%%ls\",yytext)\n"
			"# define REJECT { nstr = yyreject_w(); goto yyfussy;}\n"
			"#define yyless yyless_w\n"
			"int yyreject_w(void);\n"
			"int yyleng;\n");

		/*
		 * XCU4:
		 * If %array, yytext[] contains the token.
		 * If %pointer, yytext is a pointer to yy_tbuf[].
		 */

		if (isArray) {
			fprintf(fout,
				"#define YYISARRAY\n"
				"wchar_t yytext[YYLMAX];\n");
		} else {
			fprintf(fout,
				"wchar_t yy_tbuf[YYLMAX];\n"
				"wchar_t * yytext = yy_tbuf;\n"
				"int yytextsz = YYLMAX;\n"
				"#ifndef YYTEXTSZINC\n"
				"#define YYTEXTSZINC 100\n"
				"#endif\n");
		}
	} else {
		fprintf(fout,
			"#ifndef __cplusplus\n"
			"%s%d%s\n",
"# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==",
		ctable['\n'],
"?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)");
		fprintf(fout,
			"#else\n"
			"%s%d%s\n",
"# define lex_input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==",
		ctable['\n'],
"?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)");
		fprintf(fout,
			"#endif\n"
			"#define ECHO fprintf(yyout, \"%%s\",yytext)\n");
		if (handleeuc) {
			fprintf(fout,
"# define REJECT { nstr = yyreject_e(); goto yyfussy;}\n"
				"int yyreject_e(void);\n"
				"int yyleng;\n"
				"size_t yywleng;\n");
			/*
			 * XCU4:
			 * If %array, yytext[] contains the token.
			 * If %pointer, yytext is a pointer to yy_tbuf[].
			 */
			if (isArray) {
				fprintf(fout,
					"#define YYISARRAY\n"
					"unsigned char yytext[YYLMAX*MB_LEN_MAX];\n"
					"wchar_t yywtext[YYLMAX];\n");
			} else {
				fprintf(fout,
					"wchar_t yy_twbuf[YYLMAX];\n"
					"wchar_t yy_tbuf[YYLMAX*MB_LEN_MAX];\n"
					"unsigned char * yytext ="
					"(unsigned char *)yy_tbuf;\n"
					"wchar_t * yywtext = yy_twbuf;\n"
					"int yytextsz = YYLMAX;\n"
					"#ifndef YYTEXTSZINC\n"
					"#define YYTEXTSZINC 100\n"
					"#endif\n");
			}
		} else {
			fprintf(fout,
"# define REJECT { nstr = yyreject(); goto yyfussy;}\n"
				"int yyleng;\n");

			/*
			 * XCU4:
			 * If %array, yytext[] contains the token.
			 * If %pointer, yytext is a pointer to yy_tbuf[].
			 */
			if (isArray) {
				fprintf(fout,
					"#define YYISARRAY\n"
					"char yytext[YYLMAX];\n");
			} else {
				fprintf(fout,
					"char yy_tbuf[YYLMAX];\n"
					"char * yytext = yy_tbuf;\n"
					"int yytextsz = YYLMAX;\n"
					"#ifndef YYTEXTSZINC\n"
					"#define YYTEXTSZINC 100\n"
					"#endif\n");
			}
		}
	}
	fprintf(fout, "int yymorfg;\n");
	if (handleeuc)
		fprintf(fout, "extern wchar_t *yysptr, yysbuf[];\n");
	else
		fprintf(fout, "extern char *yysptr, yysbuf[];\n");
	fprintf(fout,
		"int yytchar;\n"
		"FILE *yyin = (FILE *)-1, *yyout = (FILE *)-1;\n"
		"#if defined (__GNUC__)\n"
		"static void _yyioinit(void) __attribute__ ((constructor));\n"
		"#elif defined (__SUNPRO_C)\n"
		"#pragma init (_yyioinit)\n"
		"#elif defined (__HP_aCC) || defined (__hpux)\n"
		"#pragma INIT \"_yyioinit\"\n"
		"#endif\n"
		"static void _yyioinit(void) {\n"
		"yyin = stdin; yyout = stdout; }\n"
		"extern int yylineno;\n"
		"struct yysvf { \n"
		"\tstruct yywork *yystoff;\n"
		"\tstruct yysvf *yyother;\n"
		"\tint *yystops;};\n"
		"struct yysvf *yyestate;\n"
		"extern struct yysvf yysvec[], *yybgin;\n");
}

#ifdef WITH_RATFOR
static void
rhd1(void)
{
	fprintf(fout,
		"integer function yylex(dummy)\n"
		"define YYLMAX 200\n"
		"define ECHO call yyecho(yytext,yyleng)\n"
		"define REJECT nstr = yyrjct(yytext,yyleng);goto 30998\n"
		"integer nstr,yylook,yywrap\n"
		"integer yyleng, yytext(YYLMAX)\n"
		"common /yyxel/ yyleng, yytext\n"
		"common /yyldat/ yyfnd, yymorf, yyprev, yybgin, yylsp, yylsta\n"
		"integer yyfnd, yymorf, yyprev, yybgin, yylsp, yylsta(YYLMAX)\n"
		"for(;;){\n"
		"\t30999 nstr = yylook(dummy)\n"
		"\tgoto 30998\n"
		"\t30000 k = yywrap(dummy)\n"
		"\tif(k .ne. 0){\n"
		"\tyylex=0; return; }\n"
		"\t\telse goto 30998\n");
}
#endif

void
phead2(void)
{
	if (!ratfor)
		chd2();
}

static void
chd2(void)
{
	fprintf(fout,
		"if (yyin == (FILE *)-1) yyin = stdin;\n"
		"if (yyout == (FILE *)-1) yyout = stdout;\n"
		"#if defined (__cplusplus) || defined (__GNUC__)\n"
	"/* to avoid CC and lint complaining yyfussy not being used ...*/\n"
		"{static int __lex_hack = 0;\n"
		"if (__lex_hack) { yyprevious = 0; goto yyfussy; } }\n"
		"#endif\n"
		"while((nstr = yylook()) >= 0)\n"
		"yyfussy: switch(nstr){\n"
		"case 0:\n"
		"if(yywrap()) return(0); break;\n");
}

void
ptail(void)
{
	if (!pflag)
#ifdef WITH_RATFOR
		ratfor ? rtail() : ctail();
#else
		ctail();
#endif
	pflag = 1;
}

static void
ctail(void)
{
	fprintf(fout,
		"case -1:\nbreak;\n"		/* for reject */
		"default:\n"
		"(void)fprintf(yyout,\"bad switch yylook %%d\",nstr);\n"
		"} return(0); }\n"
		"/* end of yylex */\n");
}

#ifdef WITH_RATFOR
static void
rtail(void)
{
	int i;
	fprintf(fout,
	"\n30998 if(nstr .lt. 0 .or. nstr .gt. %d)goto 30999\n", casecount);
	fprintf(fout,
		"nstr = nstr + 1\n"
		"goto(\n");
	for (i = 0; i < casecount; i++)
		fprintf(fout, "%d,\n", 30000+i);
	fprintf(fout,
		"30999),nstr\n"
		"30997 continue\n"
		"}\nend\n");
}
#endif

void
statistics(void)
{
	fprintf(errorf,
"%d/%d nodes(%%e), %d/%d positions(%%p), %d/%d (%%n), %ld transitions,\n",
	tptr, treesize, (int)(nxtpos-positions), maxpos, stnum + 1, nstates, rcount);
	fprintf(errorf,
	"%d/%d packed char classes(%%k), ", (int)(pcptr-pchar), pchlen);
	if (optim)
		fprintf(errorf,
		" %d/%d packed transitions(%%a), ", nptr, ntrans);
	fprintf(errorf, " %d/%d output slots(%%o)", yytop, outsize);
	putc('\n', errorf);
}
