SUBDIRS = .


INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = sipXregMocTool

sipXregMocTool_LDADD = @SIPXCOMMSERVER_LIBS@ \
		   -lmongoclient

sipXregMocTool_SOURCES =  \
	Main.cpp \
	Main.h \
	AppConfig.cpp \
	AppConfig.h \
	AppPerform.cpp \
	AppPerform.h

sipXregMocTool_CXXFLAGS =

DISTCLEANFILES = Makefile.in
