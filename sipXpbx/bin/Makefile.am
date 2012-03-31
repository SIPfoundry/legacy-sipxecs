include $(top_srcdir)/config/utility.am

libexecdir = @SIPX_LIBEXECDIR@
libexec_SCRIPTS = \
	ibm_jvm_tmpdir \
	sipx-activate-ntp \
	sipx-config-httpd-access \
	sipx-config-vxml-domain \
	initial-config

bin_SCRIPTS = \
	analyze_483s \
	ipcclean.pl \
	check-fqdn \
	sipx-archive-common \
	sipx-backup \
	sipx-restore \
	sipx-sendmail-configure \
	sipx-snapshot \
	sipx-test-apache \
	sipx-test-dnsIp \
	sipx-test-hostname \
	sipx-test-localhostConfigured \
	sipx-test-localhostShared \
	sipx-test-maxfds \
	sipx-test-rpmnew \
	sipx-test-selinux \
	sipx-test-ssl \
	sipx-test-tmp \
	sipx-zoneadmin.sh \
	sipxecs-pgcdraccess \
	sipx-package.py \
	sipx-swadmin.py \
	sipxecs-upgrade

CLEANFILES = \
	$(libexec_SCRIPTS) \
	$(bin_SCRIPTS) \
	$(daily_SCRIPTS)

EXTRA_DIST = \
	$(libexec_SCRIPTS:=.in) \
	$(bin_SCRIPTS:=.in) \
	$(daily_SCRIPTS:=.in) \
	$(messages_DATA:.mo=.po)

dailydir = @DAILYDIR@
daily_SCRIPTS = \
	voicemail_clean \
	sipx-core-clean \
	sipx-chkspace \
	sipx-cert-check

setup_LINKS = \
	010_sipx-activate-ntp \
	011_sipx-config-httpd-access \
	012_sipx-config-vxml-domain \
	020_ibm_jvm_tmpdir

NoSequenceId = $(shell echo $1 | sed -e 's|^[0-9]\{3\}_\(.*\)$$|\1|g')

$(daily_SCRIPTS) $(libexec_SCRIPTS) $(bin_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

install-exec-hook : $(addprefix $(DESTDIR)@SIPX_LIBEXECDIR@/setup.d/,$(setup_LINKS))

$(DESTDIR)@SIPX_LIBEXECDIR@/setup.d/% :
	test -d $(dir $@) || mkdir $(dir $@)
	cd  $(dir $@); \
		ln -sf ../$(call NoSequenceId,$*) $*;

