include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(schema_DATA:=.in) \
        $(proc_DATA:=.in) \
	$(cfinputs_PROMISES:=.in) \
	cfengine_stdlib.cf

cfdefaultsdir = $(SIPX_CFDATA)/defaults
cfdefaults_DATA = \
	dirs.cfdat

dirvars = \
	$(filter SIPX%DIR,$(.VARIABLES)) \
	$(filter SIPX_CF%,$(.VARIABLES))

dirs.cfdat : Makefile
	! test -f $@ || rm $@
	$(foreach V,$(dirvars), echo '=$(V)=$($(V))' >> $@;)

cfinputsdir = $(SIPX_CFINPUTS)
cfinputs_DATA = \
	$(cfinputs_PROMISES) \
	cfengine_stdlib.cf

cfinputs_PROMISES = \
	sipxsupervisord.cf \
	sipx.cf \
	promises.cf \
	sipxagent.cf

emptydirs = $(DESTDIR)$(SIPX_CFINPUTS)/plugin.d

$(emptydirs) :
	$(INSTALL) -d $@

install-data-hook : $(emptydirs)

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

$(cfinputs_PROMISES) $(schema_DATA) $(proc_DATA) : % : %.in Makefile
	@$(call SearchAndReplace, $<, $@)

CLEANFILES = $(proc_DATA) $(schema_DATA) $(cfinputs_PROMISES) $(cfdefaults_DATA)
