include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(cfinputs_PROMISES:=.in) \
	$(cflib_DATA:=.in) \
	security.sec.erb.in \
	cfengine_stdlib.cf

cfdefaultsdir = $(SIPX_CFDATA)/defaults
cfdefaults_DATA = \
	dirs.cfdat

dirvars = \
	$(filter SIPX%DIR,$(.VARIABLES)) \
	$(filter SIPX_CF%,$(.VARIABLES)) \
	$(filter SIPXPBX%,$(.VARIABLES)) \
	wwwdir \
	datadir \
	libexecdir \
	sysconfdir \
	localstatedir \
	libdir \
	bindir

dirs.cfdat : Makefile
	! test -f $@ || rm $@
	$(foreach V,$(dirvars), echo '=$(V)=$($(V))' >> $@;)

cfinputsdir = $(SIPX_CFINPUTS)
cfinputs_DATA = \
	$(cfinputs_PROMISES) \
	cfengine_stdlib.cf

cfpluginsdir = $(SIPX_CFINPUTS)/plugin.d
dist_cfplugins_DATA = \
	sipxsupervisor.cf \
	selinux.cf \
	firewall.cf \
	security.cf \
	redis.cf \
	mysql.cf \
	dhcpd.cf \
	ntpd.cf \
	phonelog.cf \
	sendmail.cf \
	logrotate.cf \
	archive.cf \
	zz_sipxlogwatcher.cf

fwconfdir = $(SIPX_CONFDIR)/firewall
dist_fwconf_DATA = \
	iptables.erb

security_DATA = \
    security.sec.erb

securitydir = $(SIPX_CONFDIR)/fail2ban
dist_security_DATA = \
	security.erb \
	$(security_DATA)

dhcpdir = $(SIPX_CONFDIR)/dhcp
dist_dhcp_DATA = \
	dhcpd.erb

ntpddir = $(SIPX_CONFDIR)/ntpd
dist_ntpd_DATA = \
	ntpd.erb

cflibdir =  $(SIPX_CFINPUTS)/plugin.d/lib
cflib_DATA = \
	commands.cf \
	upload_snapshot.cf \
	change_timezone.cf \
	snapshot.cf

cfinputs_PROMISES = \
	sipxsupervisord.cf \
	sipx.cf \
	hostname.cf \
	upload.cf \
	promises.cf \
	sipxagent.cf

emptydirs = $(DESTDIR)$(SIPX_CFINPUTS)/plugin.d

$(emptydirs) :
	$(INSTALL) -d $@

install-data-hook : $(emptydirs)

$(cflib_DATA) $(cfinputs_PROMISES) : % : %.in Makefile
	@$(call SearchAndReplace, $<, $@)

$(cflib_DATA) $(security_DATA) : % : %.in Makefile
	@$(call SearchAndReplace, $<, $@)

CLEANFILES = $(cfinputs_PROMISES) $(cfdefaults_DATA) $(cflib_DATA)
