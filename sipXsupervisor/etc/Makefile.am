include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(schema_DATA:=.in) \
        $(proc_DATA:=.in) \
	$(cfinputs_PROMISES:=.in) \
	$(cfdefaults_FILES:=.in) \
	cfengine_stdlib.cf

cfdefaultsdir = $(SIPX_CFDATA)/defaults
cfdefaults_DATA = \
	SystemDirectories.cfdat \
	$(cfdefaults_FILES)

cfdefaults_FILES = \
	sipxsupervisord.cfdat \
	mongodb.cfdat

SystemDirectoryVariables = \
	$(filter SIPX%DIR,$(.VARIABLES)) \
	$(filter SIPX_CF%,$(.VARIABLES))

SystemDirectories.dat : Makefile
	! test -f $@ || rm $@
	$(foreach V,$(SystemDirectoryVariables), echo '=$(V)=$($(V))' >> $@;)

cfinputsdir = $(SIPX_CFINPUTS)
cfinputs_DATA = \
	$(cfinputs_PROMISES) \
	cfengine_stdlib.cf

cfinputs_PROMISES = \
	sipxsupervisord.cf \
	sipx.cf \
	sipxagent.cf

cfpluginsdir = $(SIPX_CFINPUTS)/plugin.d
dist_cfplugins_DATA = \
	mongodb.cf

procdir = \
        $(SIPX_DATADIR)/process.d

proc_DATA = \
	sipxsupervisor-process.xml

schemadir = @SIPX_DATADIR@/schema
schema_DATA = \
	sipXecs-process.xsd \
	alarm.xsd \
	alarm-group.xsd

alarmcfgdir = @SIPX_CONFDIR@/alarms
dist_alarmcfg_DATA = \
	sipXalarms-config.xml

alarmdir = @SIPX_DATADIR@/alarms
dist_alarm_DATA = \
	alarm-strings.xml \
	sipXalarms-strings.xml

mibdir = @SIPX_DATADIR@/mibs
dist_mib_DATA = \
	SIPXECS-ALARM-NOTIFICATION-MIB.mib

$(cfdefaults_FILES) $(cfinputs_PROMISES) $(schema_DATA) $(proc_DATA) : % : %.in Makefile
	@$(call SearchAndReplace, $<, $@)

CLEANFILES = $(proc_DATA) $(schema_DATA) $(cfinputs_PROMISES) $(cfdefaults_FILES)
