# add -DWITH_RATFOR to your C/CPPFLAGS if you want ratfor support
# adds about 4 KB to binary size

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
LIBDIR = $(PREFIX)/lib
MANDIR = $(PREFIX)/share/man

XOBJ = main.o sub1.o sub2.o sub3.o header.o wcio.o parser.o lsearch.o

LOBJ = allprint.o libmain.o reject.o yyless.o yywrap.o \
	allprint_w.o reject_w.o yyless_w.o reject_e.o yyless_e.o

GENH = nceucform.h ncform.h nrform.h

WFLAGS = -DEUC -DJLSLEX -DWOPTION
EFLAGS = -DEUC -DJLSLEX -DEOPTION

RANLIB = ranlib
INSTALL = install

HOSTCC = $(CC)

-include config.mak

.c.o: ; $(CC) -c $(CFLAGS) $(CPPFLAGS) $(WARN) $<
.y.c: ; $(YACC) -o $@ $<

all: lex$(EXE_EXT) libl.a

form2hdr$(EXE_EXT): form2hdr.c
	$(HOSTCC) $(HOSTCFLAGS) form2hdr.c -o $@

$(GENH): %.h: % ; $(HOSTRUN) ./form2hdr$(EXE_EXT) -c $< > $@

lex$(EXE_EXT): $(XOBJ)
	$(CC) $(LDFLAGS) $(XOBJ) $(LIBS) -o $@

libl.a: $(LOBJ)
	rm -f $@
	$(AR) cru $@ $(LOBJ)
	$(RANLIB) $@

allprint_w.o: allprint.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(WARN) $(WFLAGS) allprint.c -o $@

reject_w.o: reject.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(WARN) $(WFLAGS) reject.c -o $@

yyless_w.o: yyless.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(WARN) $(WFLAGS) yyless.c -o $@

reject_e.o: reject.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(WARN) $(EFLAGS) reject.c -o $@

yyless_e.o: yyless.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(WARN) $(EFLAGS) yyless.c -o $@

install: lex lex.1 libl.a
	$(INSTALL) -D -m 755 lex $(DESTDIR)$(BINDIR)/lex
	$(INSTALL) -D -m 644 libl.a $(DESTDIR)$(LIBDIR)/libl.a
	$(INSTALL) -D -m 644 lex.1 $(DESTDIR)$(MANDIR)/man1/lex.1

clean:
	rm -f lex libl.a $(XOBJ) $(LOBJ) $(GENH) parser.c form2hdr$(EXE_EXT) core log *~

mrproper: clean

allprint.o: allprint.c
header.o: header.c ldefs.h
libmain.o: libmain.c
main.o: main.c once.h ldefs.h sgs.h $(GENH) lzcomp.h
reject.o: reject.c
sub1.o: sub1.c ldefs.h
sub2.o: sub2.c ldefs.h
sub3.o: sub3.c ldefs.h search.h
parser.o: parser.y ldefs.h
yyless.o: yyless.c
yywrap.o: yywrap.c
lsearch.o: search.h
wcio.o: ldefs.h
$(GENH): form2hdr$(EXE_EXT)

.PHONY: all clean mrproper install
# prevent GNU make from deleting parser.c after "all" finishes
# it could be needed for debugging.
.SECONDARY: parser.c
