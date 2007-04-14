noinst_LTLIBRARIES = libVXI.la 

# do not do versioning for noinst libraries

libVXI_la_CXXFLAGS = \
	@XERCES_CXXFLAGS@ \
	-DCOMPANY_DOMAIN=L\"pingtel.com\"

libVXI_la_SOURCES =  \
	Counters.cpp \
	DocumentConverter.cpp \
	DocumentModel.cpp \
	DocumentParser.cpp \
	GrammarManager.cpp \
	PromptManager.cpp \
	PropertyList.cpp \
	Scripter.cpp \
	SimpleLogger.cpp \
	VXI.cpp \
	VXI_api.cpp

noinst_HEADERS = \
	CommonExceptions.hpp \
	Counters.hpp \
	DefaultsDTD.hpp \
	Defaults.hpp \
	DocumentConverter.hpp \
	DocumentModel.hpp \
	DocumentParser.hpp \
	DocumentRep.hpp \
	DTD.hpp \
	GrammarManager.hpp \
	InternalMutex.hpp \
	PromptManager.hpp \
	PropertyList.hpp \
	Scripter.hpp \
	SimpleLogger.hpp \
	VXI.hpp \
	XMLChConverter.hpp \
	VXML.h
