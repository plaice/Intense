# *****************************************************************************
# AutoMakefile for the Intense context library.
#
# Process this file with automake to produce Makefile.in
# *****************************************************************************


INCDIR = $(srcdir)/../include

INCSUBDIR = $(srcdir)/../include/intense

INCLUDES = @INCLUDE_PREFIX@ -I$(INCDIR) -I$(INCSUBDIR) -I$(srcdir) -I.

AM_YFLAGS = -d

AM_LFLAGS = -+ -Pcontextop_yy

LEX_OUTPUT_ROOT = lex.contextop_yy

CORRECT_FLEX = $(INCSUBDIR)/correct_flex.sed

THE_SOURCES = \
    \
    AEther.cpp \
    BaseValue.cpp \
    BinaryBaseValue.cpp \
    BoundBaseValue.cpp \
    Context.cpp \
    ContextDomain.cpp \
    ContextManager.cpp \
    ContextOp.cpp \
    ContextOpLexer.lpp \
    Dimension.cpp \
    NumberBaseValue.cpp \
    SetContextDomain.cpp \
    StringBaseValue.cpp \
    StringUtil.cpp \
    \
    $(INCSUBDIR)/AEther.hpp \
    $(INCSUBDIR)/AlphaBaseValue.hpp \
    $(INCSUBDIR)/BinaryBaseValue.hpp \
    $(INCSUBDIR)/BoundBaseValue.hpp \
    $(INCSUBDIR)/BoundSerialiser.hpp \
    $(INCSUBDIR)/container_stringtok.hpp \
    $(INCSUBDIR)/ContextDomain.hpp \
    $(INCSUBDIR)/Context.hpp \
    $(INCSUBDIR)/ContextManager.hpp \
    $(INCSUBDIR)/ContextOpLexer.hpp \
    $(INCSUBDIR)/correct_FlexLexer.h \
    $(INCSUBDIR)/i \
    $(INCSUBDIR)/IntenseException.hpp \
    $(INCSUBDIR)/NumberBaseValue.hpp \
    $(INCSUBDIR)/OmegaBaseValue.hpp \
    $(INCSUBDIR)/SetContextDomain.hpp \
    $(INCSUBDIR)/slist \
    $(INCSUBDIR)/StringBaseValue.hpp \
    $(INCSUBDIR)/StringUtil.hpp \
    $(INCDIR)/intense.h \
    $(CORRECT_FLEX)

libintense_la_LDFLAGS = -version-info 2:0:0 # iface:rev:back_compat_age

lib_LTLIBRARIES = libintense.la

libintense_la_SOURCES = $(THE_SOURCES)

.lpp.cpp :
	$(LEXCOMPILE) `test -f $< || echo '$(srcdir)/'`$<
	sed -e '/^#/ s|$(LEX_OUTPUT_ROOT)\.cc|$@|' -f $(CORRECT_FLEX) \
	    $(LEX_OUTPUT_ROOT).cc >$@
	rm -f $(LEX_OUTPUT_ROOT).cc
