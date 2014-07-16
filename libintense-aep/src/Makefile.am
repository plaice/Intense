# *****************************************************************************
# AutoMakefile for the Intense Project C++ AEP 2.0 library.
# *****************************************************************************

## Process this file with automake to produce Makefile.in

INCDIR = $(srcdir)/../include

INCSUBDIR = $(srcdir)/../include/intense-aep

AM_CPPFLAGS = @INCLUDE_PREFIX@ -I$(INCDIR) -I$(INCSUBDIR) -I$(srcdir) -I.

AM_LFLAGS = -+

CORRECT_FLEX = $(INCSUBDIR)/correct_flex.sed

THE_SOURCES = \
    \
    AEPClient.cpp \
    AEPCommon.cpp \
    AEPServer.cpp \
    AEPSharedClient.cpp \
    AEPSharedServer.cpp \
    AETPClient.cpp \
    AETPServer.cpp \
    AETPLexer.lpp \
    \
    $(INCDIR)/intense-aep.hpp \
    $(INCSUBDIR)/AEPClient.hpp \
    $(INCSUBDIR)/AEPCommon.hpp \
    $(INCSUBDIR)/AEPServer.hpp \
    $(INCSUBDIR)/AETPLexer.hpp

libintense_aep_la_LDFLAGS = -version-info 2:0:0 # iface:rev:back_compat_age

lib_LTLIBRARIES = libintense-aep.la

libintense_aep_la_SOURCES = $(THE_SOURCES)

AETPLexer.cpp : AETPLexer.lpp
	$(LEXCOMPILE) `test -f $< || echo '$(srcdir)/'`$<
	sed -e '/^#/ s|lex\.aetp_yy\.cc|$@|' < lex.aetp_yy.cc >$@
	rm -f lex.aetp_yy.cc
