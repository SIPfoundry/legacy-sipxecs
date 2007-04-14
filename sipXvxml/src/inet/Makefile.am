noinst_LTLIBRARIES = libOSBinet.la

# do not do versioning for noinst libraries

libOSBinet_la_LIBADD = \
	@SIPXTACK_LIBS@

libOSBinet_la_CXXFLAGS = \
	@LIBWWW_CXXFLAGS@ \
	-DCOMPANY_DOMAIN=L\"pingtel.com\"

libOSBinet_la_SOURCES =  \
	SBHTEvtLst.cpp \
	SBHTStream.cpp \
	SBinet.cpp \
	SBinetCacheLock.cpp \
	SBinetChannel.cpp \
	SBinetCookie.cpp \
	SBinetFileStream.cpp \
	SBinetHttpStream.cpp \
	SBinetLogger.cpp \
	SBinetURL.cpp \
	SBinetValidator.cpp

noinst_HEADERS = \
	SBHTEvtLst.h \
	SBHTStream.h \
	SBinetCacheLock.h \
	SBinetChannel.h \
	SBinetCookie.h \
	SBinetInternal.h \
	SBinetLogger.hpp \
	SBinetLog.h \
	SBinetStream.h \
	SBinetString.hpp \
	SBinetURL.h \
	SBinetValidator.h
