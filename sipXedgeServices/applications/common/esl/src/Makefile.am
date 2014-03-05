SUBDIRS = .


lib_LTLIBRARIES = libsipxedge_esl.la

AM_CPPFLAGS = \
    -I$(top_srcdir)/applications/common/esl/include

AM_CFLAGS = \
    -I$(top_srcdir)/applications/common/esl/include
    
version_Current=@LIBVERSION_CURRENT@
version_Revision=@LIBVERSION_REVISION@
version_Age=@LIBVERSION_AGE@

LDADD = @BOOST_LIBS@ -lpthread

LDFLAGS = -rdynamic

libsipxedge_esl_la_LDFLAGS = -version-info @LIBVERSION_INFO@

libsipxedge_esl_la_LIBADD = $(LTLIBINTL)

#libsipxedge_esl_la_CFLAGS = -DLOCALEDIR=\"$(localedir)\"

libsipxedge_esl_la_SOURCES = \
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
