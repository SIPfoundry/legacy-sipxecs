INCLUDES = -I$(top_srcdir)/include


AM_CPPFLAGS = \
	-DTEST_DATA_DIR=\"@abs_top_srcdir@/src/test\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\" 

LDADD = \
	@SIPXUNIT_LIBS@ \
	-lmongoclient \
   ../../libsipXcommserver.la

# Define values used in local database unit tests. This only
# happens if configure was run with --enable-dbtests.


TESTS = \
	SipXApplicationTest \
	SipXApplicationTestConfigDb \
	SipXApplicationTestIni

	
	


check_PROGRAMS = $(TESTS)
			  

SipXApplicationTest_SOURCES = SipXApplicationTest.cpp
SipXApplicationTestConfigDb_SOURCES = SipXApplicationTestConfigDb.cpp
SipXApplicationTestIni_SOURCES = SipXApplicationTestIni.cpp