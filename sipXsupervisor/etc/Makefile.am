include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(schema_DATA:=.in) \
	$(cfinputs_PROMISES:=.in) \
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
	zz_sipxlogwatcher.cf

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

$(cfinputs_PROMISES) : % : %.in Makefile
	@$(call SearchAndReplace, $<, $@)

CLEANFILES = $(schema_DATA) $(cfinputs_PROMISES) $(cfdefaults_DATA)
