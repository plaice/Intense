# *****************************************************************************
# Makefile for the libintense tests.
# *****************************************************************************


TESTS = IntenseTest0 IntenseTest1 IntenseTest2 IntenseTest3 \
        IntenseTest4 IntenseTest5 IntenseTest6 IntenseTest7

check_PROGRAMS = $(TESTS)

AM_LDFLAGS = @LINK_PREFIX@ -lpthread -lintense-io -static

INCLUDES = @INCLUDE_PREFIX@ -I$(srcdir)/../include \
           -I$(srcdir)/../include/intense

OTHER_SOURCES = IntenseTest.cpp IntenseTest.hpp

IntenseTest0_SOURCES = IntenseTest0.cpp $(OTHER_SOURCES)
IntenseTest1_SOURCES = IntenseTest1.cpp $(OTHER_SOURCES)
IntenseTest2_SOURCES = IntenseTest2.cpp $(OTHER_SOURCES)
IntenseTest3_SOURCES = IntenseTest3.cpp $(OTHER_SOURCES)
IntenseTest4_SOURCES = IntenseTest4.cpp $(OTHER_SOURCES)
IntenseTest5_SOURCES = IntenseTest5.cpp $(OTHER_SOURCES)
IntenseTest6_SOURCES = IntenseTest6.cpp $(OTHER_SOURCES)
IntenseTest7_SOURCES = IntenseTest7.cpp $(OTHER_SOURCES)

IntenseTest0_LDADD = ../src/libintense.la
IntenseTest1_LDADD = ../src/libintense.la
IntenseTest2_LDADD = ../src/libintense.la
IntenseTest3_LDADD = ../src/libintense.la
IntenseTest4_LDADD = ../src/libintense.la
IntenseTest5_LDADD = ../src/libintense.la
IntenseTest6_LDADD = ../src/libintense.la
IntenseTest7_LDADD = ../src/libintense.la
