include $(top_srcdir)/config/utility.am

ssldir = @SIPX_BINDIR@/ssl-cert
ssl_SCRIPTS = \
	gen-ssl-keys.sh \
	install-cert.sh \
	check-cert.sh \
	upgrade-cert.sh \
	ca_rehash

EXTRA_DIST = \
	$(ssl_SCRIPTS:=.in)

EXTRA_REPLACE = \
	OPENSSL \
	DATE_PARSE_ARGS

CLEANFILES = $(ssl_SCRIPTS)

authoritiesdir = @SIPX_CONFDIR@/ssl/authorities
dist_authorities_DATA = verisignclass3ca.crt

dist_man1_MANS = gen-ssl-keys.1

$(ssl_SCRIPTS) : % : %.in
	@$(call SearchAndReplace,$<,$@,$(EXTRA_REPLACE))

