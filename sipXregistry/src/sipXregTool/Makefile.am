SUBDIRS = .


INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = sipXregTool

sipXregTool_LDADD = @SIPXCOMMSERVER_LIBS@ \
		   -lmongoclient

sipXregTool_SOURCES =  \
	Main.cpp \
	Main.h \
	AppConfig.cpp \
	AppConfig.h \
	AppPerform.cpp \
	AppPerform.h

sipXregTool_CXXFLAGS =

DISTCLEANFILES = Makefile.in
