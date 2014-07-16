# *****************************************************************************
# AutoMakefile for the Intense Project C++ I/O library.
# *****************************************************************************

## Process this file with automake to produce Makefile.in

INCDIR = $(srcdir)/../include
INCSUBDIR = $(srcdir)/../include/intense-io
INCLUDES = -I$(INCDIR) -I$(INCSUBDIR) -I$(srcdir) -I.

THE_SOURCES = \
    \
    BinaryBaseSerialiser.cpp \
    ByteCountFileStream.cpp \
    FileStream.cpp \
    LogStream.cpp \
    XDRBaseSerialiser.cpp \
    \
    $(INCDIR)/intense-io.hpp \
    $(INCSUBDIR)/intense_io_config.hpp \
    $(INCSUBDIR)/BinaryBaseSerialiser.hpp \
    $(INCSUBDIR)/BaseSerialiser.hpp \
    $(INCSUBDIR)/ByteCountFileStream.hpp \
    $(INCSUBDIR)/FileStream.hpp \
    $(INCSUBDIR)/LogStream.hpp \
    $(INCSUBDIR)/XDRBaseSerialiser.hpp

libintense_io_la_LDFLAGS = -version-info 1:0:0 # iface:rev:back_compat_age

lib_LTLIBRARIES = libintense-io.la
libintense_io_la_SOURCES = $(THE_SOURCES)
