include $(top_srcdir)/config/utility.am

confdir = @SIPX_CONFDIR@
dist_conf_DATA = \
	mime.types

wwwconfdir = @SIPX_CONFDIR@
wwwconf_DATA = \
	httpd.conf \
	httpd-sipxchange-common-ssl.conf \
	httpd-dav.conf  \
	httpd-sipxchange-common.conf.in

EXTRA_REPLACE = \
	SERVICEDIR \
	SYSLOG_CMD \
	SYSLOG_LIBS \
	SYSLOG_ARGS \
	APACHE2_HOST_ACCESS \
	APACHE2_MOD_ACCESS

$(logrotate_DATA) $(wwwconf_DATA) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@,EXTRA_REPLACE)

CLEANFILES = \
	$(wwwconf_DATA) \
	$(logrotate_DATA)

EXTRA_DIST = \
	$(wwwconf_DATA:=.in) \
	$(logrotate_DATA:=.in)

emptydirs = \
	$(DESTDIR)@SIPX_CONFDIR@/httpd.conf.d \
	$(DESTDIR)$(wwwdir)/var \
	$(DESTDIR)$(wwwdir)/conf \
	$(DESTDIR)$(wwwdir)/webdav \
	$(DESTDIR)@SIPX_LOGDIR@/phone

$(emptydirs) :
	$(INSTALL) -d $@

install-data-hook : $(emptydirs)

logrotatedir = $(sysconfdir)/logrotate.d
logrotate_DATA = \
	sipxchange
