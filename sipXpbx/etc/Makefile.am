include $(top_srcdir)/config/utility.am

confdir = @SIPX_CONFDIR@
dist_conf_DATA = \
	mime.types \
	phonelog.conf

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

$(logrotate_DATA) $(service_SCRIPTS) $(wwwconf_DATA) : % : %.in
	@$(call SearchAndReplace,$<,$@,EXTRA_REPLACE)

CLEANFILES = \
	$(wwwconf_DATA) \
	$(service_SCRIPTS) \
	$(logrotate_DATA)

EXTRA_DIST = \
	$(wwwconf_DATA:=.in) \
	$(service_SCRIPT:=.in) \
	$(logrotate_DATA:=.in)

servicedir = @SERVICEDIR@
service_SCRIPTS = \
	sipxecs \
	phonelogd

httpdconfddir = @SIPX_CONFDIR@/httpd.conf.d
wwwvardir = $(wwwdir)/var
webdavdir = $(wwwdir)/webdav
phonelogdir = @SIPX_LOGDIR@/phone

logrotatedir = $(sysconfdir)/logrotate.d
logrotate_DATA = \
	sipxchange
