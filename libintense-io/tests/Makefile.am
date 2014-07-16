# *****************************************************************************
# Makefile for the libintense-io tests.
# *****************************************************************************

TESTS = IntenseIOTest0 BinarySerialiserTestSource

check_PROGRAMS = $(TESTS)

AM_LDFLAGS = -lpthread @XDR_LIBS@ -static

INCLUDES = -I$(srcdir)/../include -I$(srcdir)/../include/intense

OTHER_SOURCES = IntenseIOTest.cpp IntenseIOTest.hpp Test.cpp Test.hpp

IntenseIOTest0_SOURCES = IntenseIOTest0.cpp $(OTHER_SOURCES)

IntenseIOTest0_LDADD = ../src/libintense-io.la

BinarySerialiserTestSource_SOURCES = BinarySerialiserTestSource.cpp

BinarySerialiserTestSource_LDADD = ../src/libintense-io.la

nobase_include_HEADERS = intense-io/CompatibleArchitecture.hpp

intense-io/CompatibleArchitecture.hpp : BinarySerialiserTestSource.out
	./find_compatible_arch.sh $< > $@

BinarySerialiserTestSource.out : BinarySerialiserTestSource
	./BinarySerialiserTestSource

CLEANFILES = intense-io/CompatibleArchitecture.hpp BinarySerialiserTestSource.out
