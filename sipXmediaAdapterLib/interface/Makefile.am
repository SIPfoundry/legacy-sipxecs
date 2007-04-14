## Process this file with automake to produce Makefile.in

nobase_includeHEADERS_INSTALL = $(INSTALL) -D -p -c -m 644

nobase_include_HEADERS =  \
    mi/CpMediaInterface.h \
    mi/CpMediaInterfaceFactoryImpl.h \
    mi/CpMediaInterfaceFactoryFactory.h \
    mi/CpMediaInterfaceFactory.h
    
noinst_LTLIBRARIES = libmediainterface.la

libmediainterface_la_LIBADD =  \
	@SIPXMEDIA_LIBS@

libmediainterface_la_SOURCES =  \
    CpMediaInterface.cpp \
    CpMediaInterfaceFactoryImpl.cpp \
    CpMediaInterfaceFactory.cpp \
    mi/CpMediaInterface.h \
    mi/CpMediaInterfaceFactoryImpl.h \
    mi/CpMediaInterfaceFactoryFactory.h \
    mi/CpMediaInterfaceFactory.h



