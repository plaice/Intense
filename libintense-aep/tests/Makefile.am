# *****************************************************************************
# Makefile for the libintense-aep tests.
# *****************************************************************************

TESTS = IntenseAEPTest0 IntenseAEPTest1 IntenseAEPTest2 IntenseAEPTest3

check_PROGRAMS = $(TESTS)

AM_LDFLAGS = @LINK_PREFIX@ @LIBS@ -static

AM_CPPFLAGS = @INCLUDE_PREFIX@ -I$(srcdir)/../include \
           -I$(srcdir)/../include/intense-aep -I../include

OTHER_SOURCES = IntenseAEPTest.cpp IntenseAEPTest.hpp Test.cpp Test.hpp

IntenseAEPTest0_SOURCES = IntenseAEPTest0.cpp $(OTHER_SOURCES)

IntenseAEPTest1_SOURCES = IntenseAEPTest1.cpp $(OTHER_SOURCES)

IntenseAEPTest2_SOURCES = IntenseAEPTest2.cpp $(OTHER_SOURCES)

IntenseAEPTest3_SOURCES = IntenseAEPTest3.cpp $(OTHER_SOURCES)

IntenseAEPTest0_LDADD = ../src/libintense-aep.la -lintense -lintense-io

IntenseAEPTest1_LDADD = ../src/libintense-aep.la -lintense -lintense-io

IntenseAEPTest2_LDADD = ../src/libintense-aep.la -lintense -lintense-io

IntenseAEPTest3_LDADD = ../src/libintense-aep.la -lintense -lintense-io
