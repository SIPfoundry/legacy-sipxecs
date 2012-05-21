include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(cflib_DATA:=.in)

cfinputsdir = $(SIPX_CFINPUTS)/plugin.d
dist_cfinputs_DATA = \
	snmp.cf \
	snmptrap.cf

cflibdir = $(SIPX_CFINPUTS)/plugin.d/lib
cflib_DATA = upload_alarm_log.cf

confdir = $(SIPX_CONFDIR)/snmp
dist_conf_DATA = \
	email.erb \
	sms.erb

$(cflib_DATA) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

mibsdir = $(datadir)/snmp/mibs
dist_mibs_DATA = \
	SIPXECS-ALARM-NOTIFICATION-MIB.txt

