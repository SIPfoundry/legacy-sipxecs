## Process this file with automake to produce Makefile.in

# Only go into test subdir if --with-test-cse was specified

if ENABLE_CSE_TESTS
TEST_CSE_OPT = test
endif

SUBDIRS = . $(TEST_CSE_OPT)

noinst_LTLIBRARIES = libcse.la

libcse_la_LIBADD =  \
	@SIPXCOMMSERVER_LIBS@

libcse_la_SOURCES =  \
    CallStateEventBuilder.h \
    CallStateEventBuilder.cpp \
    CallStateEventBuilder_XML.h \
    CallStateEventBuilder_XML.cpp \
    CallStateEventBuilder_DB.h \
    CallStateEventBuilder_DB.cpp \
    CallStateEventWriter.h \
    CallStateEventWriter.cpp \
    CallStateEventWriter_XML.h \
    CallStateEventWriter_XML.cpp \
    CallStateEventWriter_DB.h \
    CallStateEventWriter_DB.cpp



