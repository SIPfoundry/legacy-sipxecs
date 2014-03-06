SUBDIRS = .


lib_LTLIBRARIES = libsipXesl.la

AM_CPPFLAGS = \
    -I$(top_srcdir)/applications/common/esl/include

AM_CFLAGS = \
    -I$(top_srcdir)/applications/common/esl/include
    
INCLUDES = -I$(top_srcdir)/include

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=2
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

LDADD =  \
        -lboost_system-mt \
        -lboost_thread-mt \
        -lboost_filesystem-mt \
        -lpthread

LDFLAGS = -rdynamic

libsipXesl_la_LDFLAGS = -version-info ${version_Current}:${version_Revision}:${version_Age}

libsipXesl_la_LIBADD = $(LTLIBINTL)

#libsipXesl_la_CFLAGS = -DLOCALEDIR=\"$(localedir)\"

libsipXesl_la_SOURCES = \
        cc.h \
        esl.h \
        esl_buffer.h \
        esl_config.h \
        esl_event.h \
        esl_json.h \
        esl_oop.h \
        esl_threadmutex.h \
	EslConnection.cpp \
	EslEvent.cpp \
	EslListener.cpp \
	esl.c \
	esl_buffer.c \
	esl_config.c \
	esl_event.c \
	esl_json.c \
	esl_oop.cpp \
	esl_threadmutex.c
