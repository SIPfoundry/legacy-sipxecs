INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/include 

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = testsuite

check_PROGRAMS = testsuite

testsuite_CXXFLAGS = @CPPUNIT_CFLAGS@ \
    -DTEST_DATA_DIR=\"@abs_top_srcdir@/src/test/\" \
    -DTEST_WORK_DIR=\"@abs_top_builddir@/src/test/work/\" 

testsuite_LDADD = \
   @SIPXCOMMSERVERTEST_LIBS@ \
   ../libsipXsupervisor.la

testsuite_SOURCES = \
	 ProcessStateTest.cpp \
    ProcessDefinitionParserTest.cpp

EXTRA_DIST = \
    processDef/badconfigaccess.xml \
    processDef/badexecute.xml \
    processDef/badfileattr.xml \
    processDef/badfile.xml \
    processDef/badlog.xml \
    processDef/badNamespace.xml \
    processDef/badpid.xml \
    processDef/badrequired.xml \
    processDef/badRoot.xml \
    processDef/emptycommands.xml \
    processDef/emptyexecute.xml \
    processDef/emptyfile.xml \
    processDef/emptylog.xml \
    processDef/emptyname.xml \
    processDef/emptypid.xml \
    processDef/emptyversion.xml \
    processDef/namenottext.xml \
    processDef/nocommands.xml \
    processDef/noconfigtest.xml \
    processDef/noexecute.xml \
    processDef/noname.xml \
    processDef/nostart.xml \
    processDef/nostatus.xml \
    processDef/notalog.xml \
    processDef/notaparameter.xml \
    processDef/notresources.xml \
    processDef/notwellformed.xml \
    processDef/noversion.xml \
    processDef/unknownresource.xml \
    processDef/unknownstatus1.xml \
    processDef/unknownstatus2.xml \
    processDef/versionnottext.xml

clean: clean-workdir

.PHONY: clean-workdir
clean-workdir:
	rm -rf @abs_top_builddir@/src/test/work
