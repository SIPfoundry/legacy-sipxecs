INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/include

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = testsuite alarm_testsuite

# disabled for mongo port
#check_PROGRAMS = testsuite alarm_testsuite
check_PROGRAMS =

testsuite_CXXFLAGS = @CPPUNIT_CFLAGS@ \
    -DTEST_DATA_DIR=\"@abs_top_srcdir@/src/test/\" \
    -DTEST_WORK_DIR=\"@abs_top_builddir@/src/test/work/\"

testsuite_LDADD = \
   @SIPXCOMMSERVERTEST_LIBS@ \
   ../libsipXsupervisor.la \
    ../libsipXalarms.la

testsuite_SOURCES = \
    SipxProcessDefinitionParserTest.cpp \
	 SipxProcessVersionsTest.cpp \
	 SipxProcessStateTest.cpp \
	 SipxCommandTest.cpp \
	 SipxResourceUseTest.cpp \
	 FileDirResourceTest.cpp

alarm_testsuite_CXXFLAGS = \
    -DTEST_DATA_DIR=\"@abs_top_srcdir@/src/test\" \
    -DTEST_WORK_DIR=\"@abs_builddir@\" \
    -DETC_DIR=\"@abs_top_srcdir@/etc\" \
    -DSIPX_CONFDIR=\"@SIPX_CONFDIR@\" \
    -DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
    -DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
    -DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
    -DSIPX_DBDIR=\"@SIPX_DBDIR@\"

alarm_testsuite_LDADD = \
    @SIPXCOMMSERVERTEST_LIBS@ \
    @SIPXCOMMSERVER_LIBS@ \
    ../libsipXalarms.la

alarm_testsuite_SOURCES = \
    AlarmServerTest.cpp \
    AlarmLocalizationTest.cpp

EXTRA_DATA = \
    alarm-server/alarm-config.xml \
    alarm-server/alarm-groups.xml \
    alarm-server/test-alarms-config.xml \
    alarm-server/test-alarms-strings.xml \
    alarm-localization/alarm-config.xml \
    alarm-localization/alarm-groups.xml \
    alarm-localization/test-localization-config.xml \
    alarm-localization/test-localization-strings.xml \
    alarm-localization/test-localization-strings_fr.xml \
    processDef/badconfigaccess.xml \
    processDef/baddiraccess.xml \
    processDef/badexecute.xml \
    processDef/badfileattr.xml \
    processDef/badfile.xml \
    processDef/badlog.xml \
    processDef/badNamespace.xml \
    processDef/badpatterns.xml \
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
    processDef/nopath.xml \
    processDef/nopattern.xml \
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
    processDef/versionnottext.xml \
	 fileDir/firstprocess.xml \
	 resourceUse/firstprocess.xml \
    resourceUse/secondprocess.xml \
	 commandDef/goodcommand.xml \
	 processState/newprocess.xml \
	 processVersions/someprocess.xml

EXTRA_DIST = \
    $(EXTRA_DATA)

clean: clean-workdir

.PHONY: clean-workdir
clean-workdir:
	rm -rf @abs_top_builddir@/src/test/work
