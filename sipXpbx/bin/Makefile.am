include $(top_srcdir)/config/utility.am

libexecdir = @SIPX_LIBEXECDIR@
libexec_SCRIPTS = \
	ibm_jvm_tmpdir \
	sipx-activate-ntp \
	sipx-config-httpd-access \
	sipx-config-vxml-domain \
	initial-config \
	sipx-strip-db-secrets.pl \
	sipxkeystoregen

pylibexec_SRC = \
	sipxecs_setup_common.py

pylibexecdir = @SIPX_LIBEXECDIR@
# install python source in case it needs to be modified
pylibexec_DATA = \
	$(pylibexec_SRC) \
	$(pylibexec_SRC=.c)

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
	sipxecs-setup \
	sipxecs-setup-system \
	sipxecs-upgrade

CLEANFILES = \
	$(pylibexec_SRC) \
	$(libexec_SCRIPTS) \
	$(bin_TEMPLATES) \
	$(daily_SCRIPTS)

EXTRA_DIST = \
	$(pylibexec_SRC:=.in) \
	$(libexec_SCRIPTS:=.in) \
	$(bin_TEMPLATES:=.in) \
	$(daily_SCRIPTS:=in) \
	$(messages_DATA:.mo=.po)

messagesdir = \
	$(SIPX_DATADIR)/locale/en_US/LC_MESSAGES

messages_DATA = \
	sipxecs-setup.mo

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
	013_sipxkeystoregen \
	020_ibm_jvm_tmpdir

NoSequenceId = $(shell echo $1 | sed -e 's|^[0-9]\{3\}_\(.*\)$$|\1|g')

$(pylibexec_SRC) $(daily_SCRIPTS) $(libexec_SCRIPTS) $(bin_SCRIPTS) : % : %.in
	@$(call SearchAndReplace,$<,$@)

%.mo : %.po
	msgfmt --output-file=$@ $<

%.pyc : %.py
	python -m py_compile $<

install-exec-hook : $(addprefix $(DESTDIR)@SIPX_LIBEXECDIR@/setup.d/,$(setup_LINKS))

$(DESTDIR)@SIPX_LIBEXECDIR@/setup.d/% :
	test -d $(dir $*) || mkdir $(dir $*)
	cd $(DESTDIR)@SIPX_LIBEXECDIR@/setup.d; \
		ln -sf ../$(call NoSequenceId,$*) $*;

