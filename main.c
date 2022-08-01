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
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/* Copyright (c) 1988 AT&T */
/* All Rights Reserved */

/* Copyright 1976, Bell Telephone Laboratories, Inc. */

/*	from OpenSolaris "main.c	6.16	05/06/08 SMI"	*/

/*
 * Portions Copyright (c) 2005 Gunnar Ritter, Freiburg i. Br., Germany
 *
 * Sccsid @(#)main.c	1.9 (gritter) 11/26/05
 */

#include <wchar.h>
#include <string.h>
#include "once.h"
#include "sgs.h"
#include <locale.h>
#include <limits.h>
#include <unistd.h>

#include "nceucform.h"
#include "ncform.h"
#ifdef WITH_RATFOR
#include "nrform.h"
#endif

#define LZ_DECOMPRESSOR
#define LZ_UNSAFE
#include "lzcomp.h"
static char* decomp(const unsigned char *in,
	unsigned inlen, unsigned outlen) {
	char *buf = myalloc(1, outlen);
	lz_uncompress(in, buf, inlen);
	return buf;
}
#define DECOMP(X) decomp((X).data, (X).clen, (X).ulen)

#ifdef WHOLE_PROGRAM
#include "sub1.c"
#include "sub2.c"
#include "sub3.c"
#include "header.c"
#include "parser.c"
#include "lsearch.c"
#endif

static wchar_t  L_INITIAL[] = {'I', 'N', 'I', 'T', 'I', 'A', 'L', 0};

static void get1core(void);
static void free1core(void);
static void get2core(void);
static void free2core(void);
static void get3core(void);
#ifdef DEBUG
static void free3core(void);
#endif

int
main(int argc, char **argv)
{
	int i;
	int c;
	Boolean eoption = 0, woption = 0;
	const char *driver = 0;
        const char *fnout = "yy.lex.c";

	sargv = argv;
	sargc = argc;
	errorf = stderr;
	setlocale(LC_CTYPE, "");
	while ((c = getopt(argc, argv,
#ifdef DEBUG
                "dy"
#endif
                "ctvneiwVQ:o:")) != EOF) {
		switch (c) {
#ifdef DEBUG
			case 'd':
				debug++;
				break;
			case 'y':
				yydebug = TRUE;
				break;
#endif
			case 'V':
				fprintf(stderr, "lex:%s, %s\n", pkg, rel);
				return 0;
			case 'Q':
				v_stmp = optarg;
				if (*v_stmp != 'y' && *v_stmp != 'n')
					error(
					"lex: -Q should be followed by [y/n]");
				break;
			case 'o':
				fout = fopen(optarg, "w");
				if (!fout)
					error(
					"lex: could not open %s for writing",
					optarg);
				break;
			case 'c':
#ifdef WITH_RATFOR
				ratfor = FALSE;
#endif
				break;
			case 't':
				fout = stdout;
				break;
			case 'v':
				report = 1;
				break;
			case 'n':
				report = 0;
				break;
			case 'w':
			case 'W':
				woption = 1;
				handleeuc = 1;
				widecio = 1;
				break;
			case 'e':
			case 'E':
				eoption = 1;
				handleeuc = 1;
				widecio = 0;
				break;
			case 'i':
				caseless = TRUE;
				break;
			default:
				fprintf(stderr,
				"Usage: lex [-eiwctvnV] [-o outfile] [-Q(y/n)] [file]\n");
				exit(1);
		}
	}
	if (woption && eoption) {
		error(
		"You may not specify both -w and -e simultaneously.");
	}
	no_input = argc - optind;
	if (no_input) {
		/* XCU4: recognize "-" file operand for stdin */
		if (strcmp(argv[optind], "-") == 0)
			fin = stdin;
		else {
			fin = fopen(argv[optind], "r");
			if (fin == NULL)
				error(
				"Can't open input file -- %s", argv[optind]);
		}
	} else
		fin = stdin;
        if(!fout) {
                fout = fopen(fnout, "w");
                if (!fout)
                        error(
                        "lex: could not open %s for writing",
                        fnout);
        }

	/* may be gotten: def, subs, sname, schar, ccl, dchar */
	gch();

	/* may be gotten: name, left, right, nullstr, parent */
	get1core();

	scopy(L_INITIAL, sp);
	sname[0] = sp;
	sp += slength(L_INITIAL) + 1;
	sname[1] = 0;

	/* XCU4: %x exclusive start */
	exclusive[0] = 0;

	if (!handleeuc) {
		/*
		 * Set ZCH and ncg to their default values
		 * as they may be needed to handle %t directive.
		 */
		ZCH = ncg = NCH; /* ncg behaves as constant in this mode. */
	}

	/* may be disposed of: def, subs, dchar */
	if (yyparse())
		exit(1);	/* error return code */

	if (handleeuc) {
		ncg = ncgidtbl * 2;
		ZCH = ncg;
		if (ncg >= MAXNCG)
			error(
			"Too complex rules -- requires too many char groups.");
		sortcgidtbl();
	}
	repbycgid(); /* Call this even in ASCII compat. mode. */

	/*
	 * maybe get:
	 *		tmpstat, foll, positions, gotof, nexts,
	 *		nchar, state, atable, sfall, cpackflg
	 */
	free1core();
	get2core();
	ptail();
	mkmatch();
#ifdef DEBUG
	if (debug)
		pccl();
#endif
	sect  = ENDSECTION;
	if (tptr > 0)
		cfoll(tptr-1);
#ifdef DEBUG
	if (debug)
		pfoll();
#endif
	cgoto();
#ifdef DEBUG
	if (debug) {
		printf("Print %d states:\n", stnum + 1);
		for (i = 0; i <= stnum; i++)
			stprt(i);
	}
#endif
	/*
	 * may be disposed of:
	 *		positions, tmpstat, foll, state, name,
	 *		left, right, parent, ccl, schar, sname
	 * maybe get:	 verify, advance, stoff
	 */
	free2core();
	get3core();
	layout();
	/*
	 * may be disposed of:
	 *		verify, advance, stoff, nexts, nchar,
	 *		gotof, atable, ccpackflg, sfall
	 */

#ifdef DEBUG
	free3core();
#endif
	if (handleeuc) {
#ifdef WITH_RATFOR
		if (ratfor)
			error("Ratfor is not supported by -w or -e option.");
#endif
		driver = DECOMP(nceucform);
	}
	else
#ifdef WITH_RATFOR
		driver = ratfor ? DECOMP(nrform) : DECOMP(ncform);
#else
		driver = DECOMP(ncform);
#endif

	fprintf(fout, "%s", driver);
	fclose(fout);
	if (report == 1)
		statistics();
	fclose(stdout);
	fclose(stderr);
	return (0);	/* success return code */
}

static void
get1core(void)
{
	ccptr =	ccl = myalloc(CCLSIZE, sizeof (*ccl));
	pcptr = pchar = myalloc(pchlen, sizeof (*pchar));
	def = myalloc(DEFSIZE, sizeof (*def));
	subs = myalloc(DEFSIZE, sizeof (*subs));
	dp = dchar = myalloc(DEFCHAR, sizeof (*dchar));
	sname = myalloc(STARTSIZE, sizeof (*sname));
	/* XCU4: exclusive start array */
	exclusive = myalloc(STARTSIZE, sizeof (*exclusive));
	sp = schar = myalloc(STARTCHAR, sizeof (*schar));
	if (ccl == 0 || def == 0 ||
	    pchar == 0 || subs == 0 || dchar == 0 ||
	    sname == 0 || exclusive == 0 || schar == 0)
		error("Too little core to begin");
}

static void
free1core(void)
{
	free(def);
	free(subs);
	free(dchar);
}

static void
get2core(void)
{
	int i;
	gotof = myalloc(nstates, sizeof (*gotof));
	nexts = myalloc(ntrans, sizeof (*nexts));
	nchar = myalloc(ntrans, sizeof (*nchar));
	state = myalloc(nstates, sizeof (*state));
	atable =myalloc(nstates, sizeof (*atable));
	sfall = myalloc(nstates, sizeof (*sfall));
	cpackflg = myalloc(nstates, sizeof (*cpackflg));
	tmpstat = myalloc(tptr+1, sizeof (*tmpstat));
	foll = myalloc(tptr+1, sizeof (*foll));
	nxtpos = positions = myalloc(maxpos, sizeof (*positions));
	if (tmpstat == 0 || foll == 0 || positions == 0 ||
	    gotof == 0 || nexts == 0 || nchar == 0 ||
	    state == 0 || atable == 0 || sfall == 0 || cpackflg == 0)
		error("Too little core for state generation");
	for (i = 0; i <= tptr; i++)
		foll[i] = 0;
}

static void
free2core(void)
{
	free(positions);
	free(tmpstat);
	free(foll);
	free(name);
	free(left);
	free(right);
	free(parent);
	free(nullstr);
	free(state);
	free(sname);
	/* XCU4: exclusive start array */
	free(exclusive);
	free(schar);
	free(ccl);
}

static void
get3core(void)
{
	verify = myalloc(outsize, sizeof (*verify));
	advance = myalloc(outsize, sizeof (*advance));
	stoff = myalloc(stnum+2, sizeof (*stoff));
	if (verify == 0 || advance == 0 || stoff == 0)
		error("Too little core for final packing");
}

#ifdef DEBUG
static void
free3core(void)
{
	free(advance);
	free(verify);
	free(stoff);
	free(gotof);
	free(nexts);
	free(nchar);
	free(atable);
	free(sfall);
	free(cpackflg);
}
#endif

void *
myalloc(int a, int b)
{
	void *i;
	i = calloc(a,  b);
	if (i == NULL)
		warning("calloc returns a 0");
	return (i);
}

void
yyerror(char *s)
{
	fprintf(stderr,
		"\"%s\":line %d: Error: %s\n", sargv[optind], yyline, s);
}

#ifdef WHOLE_PROGRAM
#include "wcio.c"
#endif
