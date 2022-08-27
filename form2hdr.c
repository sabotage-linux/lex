/* (C) 2019 rofl0r */
/* released into the public domain, or at your choice 0BSD or WTFPL */

#ifdef __POCC__
/* pelles C fails to use string literal as unsigned char[] destination... */
#define SIMPLE_OUTPUT
/* required to get fmemopen() prototype from stdio.h */
#define __STDC_WANT_LIB_EXT2__ 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#define LZ_COMPRESSOR
#include "lzcomp.h"

static int usage() {
	printf( "form2hdr - converts a form file into a C string\n"
		"the output is written to stdout.\n"
		"usage: form2hdr [-c] filename\n"
		"if -c is given, the input will be LZ77 compressed\n");
	return 1;
}

static int skip_header(FILE *f) {
	unsigned char buf[1024];
	while(fgets(buf, sizeof buf, f)) {
		if(strstr(buf, "START_INCLUDE")) return 1;
	}
	return 0;
}

static int isprintable(int ch) {
#define PRINTABLE " ;,:.^-+=*/%|&[](){}<>#_"
	return ch > 0 && (isdigit(ch) || isalpha(ch) || strchr(PRINTABLE, ch));
}

static int escape_char(int ch, char buf[5], int *dirty) {
	int len = 0;
	*dirty = 0;
	buf[len++] = '\\';
	switch(ch) {
		case '\a': /* 0x07 */
			buf[len++] = 'a'; break;
		case '\b': /* 0x08 */
			buf[len++] = 'b'; break;
		case '\t': /* 0x09 */
			buf[len++] = 't'; break;
		case '\n': /* 0x0a */
			buf[len++] = 'n'; break;
		case '\v': /* 0x0b */
			buf[len++] = 'v'; break;
		case '\f': /* 0x0c */
			buf[len++] = 'f'; break;
		case '\r': /* 0x0d */
			buf[len++] = 'r'; break;
		case '\"': /* 0x22 */
			buf[len++] = '\"'; break;
		case '\'': /* 0x27 */
			buf[len++] = '\''; break;
		case '\?': /* 0x3f */
			buf[len++] = '\?'; break;
		case '\\': /* 0x5c */
			buf[len++] = '\\'; break;
		default:
			if(isprintable(ch)) buf[0] = ch;
			else {
				if (ch <= 077) *dirty = 1;
				len += sprintf(buf+len, "%o", ch);
			}
	}
	buf[len] = 0;
	return len;
}

int main(int argc, char** argv) {
	int c, compress = 0;
	int f_arg = 1;
	while((c = getopt(argc, argv, "c")) != EOF) switch(c) {
		case 'c': compress = 1; f_arg++; break;
		default: usage();
	}
	unsigned cpl = 77;
	FILE *f = fopen(argv[f_arg], "rb");
	if(!f) { perror("fopen"); return 1; }
	if(!skip_header(f)) {
		fprintf(stderr, "error: form start marker %s not found!\n", "START_INCLUDE");
		return 1;
	}
	{
		off_t end, start = ftello(f);
		size_t insize, outsize;
		fseeko(f, 0, SEEK_END);
		end = ftello(f);
		fseeko(f, start, SEEK_SET);
		insize = end - start;
		outsize = compress ? (insize * 1.04) + 2 : insize;

		if(compress) {
			unsigned char *inb  = malloc(insize);
			unsigned char *outb = malloc(outsize);
			fread(inb, 1, insize, f);
			outsize = lz_compress(inb, outb, insize);
#ifdef DEBUG
			dprintf(2, "ucomp size: %zu; comp size: %zu\n", insize, outsize);
			{ FILE *foo = fopen("debug", "w"); fwrite(outb, 1, outsize, foo); fclose(foo); }
#endif
			fclose(f);
			f = fmemopen(outb, outsize, "rb");
		}

		char *p = strrchr(argv[f_arg], '/');
		if(!p) p = argv[f_arg];
		else p++;
		printf( "const struct { unsigned clen, ulen;\n"
			"const unsigned char data[%zu]; } %s = { %zu, %zu,\n",
			outsize, p, outsize, insize);
	}

	int ch, dirty;
	unsigned cnt = 0;

#ifndef SIMPLE_OUTPUT
	while((ch = fgetc(f)) != EOF) {
		if(cnt == 0) { printf("\""); dirty = 0; }
		char buf[5];
		int cdirty;
		cnt += escape_char(ch, buf, &cdirty);
		// dirty and cdirty: ok
		// dirty and not cdirty: ok if buf[0] not in '0'..'7'
		if(dirty && !cdirty && buf[0] >= '0' && buf[0] <= '7') {
			printf("\"\"");
			cnt += 2;
		}
		dirty = cdirty;
		printf("%s", buf);
		if(cnt >= cpl) {
			printf("\"\n");
			cnt = 0;
		}
	}
	if(cnt) printf("\"\n");
#else
	printf("{\n");
	while((ch = fgetc(f)) != EOF) {
		++cnt;
		printf("%u,", ch);
		if(cnt % cpl == 0) printf("\n");
	}
	printf("}\n");
#endif
	printf("};\n");

	fclose(f);
	return 0;
}

