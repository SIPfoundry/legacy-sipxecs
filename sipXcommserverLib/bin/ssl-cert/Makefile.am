include $(top_srcdir)/config/subdir.am

SCRIPT_TEMPLATES = \
	gen-ssl-keys.sh \
	install-cert.sh \
	check-cert.sh \
	upgrade-cert.sh \
	ca_rehash

authoritiesdir = @SIPX_CONFDIR@/ssl/authorities
dist_authorities_DATA = verisignclass3ca.crt

dist_man1_MANS = gen-ssl-keys.1

EXTRA_DIST = \
	$(foreach sipxScript,$(SCRIPT_TEMPLATES),$(sipxScript).in)

all: localize-sipx-scripts

.PHONY: localize-sipx-scripts
localize-sipx-scripts: $(foreach sipxScript,$(SCRIPT_TEMPLATES),$(sipxScript))

$(foreach sipxScript,$(SCRIPT_TEMPLATES),$(sipxScript)) : % : $(srcdir)/%.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) \
		-e 's,@OPENSSL\@,@OPENSSL@,g' \
		-e 's,@DATE_PARSE_ARGS\@,@DATE_PARSE_ARGS@,g' \
		$< > $@
	chmod +x $@

install-exec-hook : ssl-dirs install-sipx-scripts

.PHONY: ssl-dirs
ssl-dirs : $(DESTDIR)@SIPX_CONFDIR@/ssl $(DESTDIR)@SIPX_CONFDIR@/ssl/authorities

$(DESTDIR)@SIPX_CONFDIR@/ssl:
	$(INSTALL) -d -m 0755 $(DESTDIR)@SIPX_CONFDIR@/ssl

$(DESTDIR)@SIPX_CONFDIR@/ssl/authorities:
	$(INSTALL) -d -m 0755 $(DESTDIR)@SIPX_CONFDIR@/ssl/authorities

.PHONY: install-sipx-scripts
install-sipx-scripts: $(DESTDIR)$(bindir)/ssl-cert/check-cert.sh
install-sipx-scripts: $(foreach sipxScript,$(SCRIPT_TEMPLATES),$(DESTDIR)$(bindir)/ssl-cert/$(sipxScript))

$(foreach sipxScript,$(SCRIPT_TEMPLATES),$(DESTDIR)$(bindir)/ssl-cert/$(sipxScript)) : $(DESTDIR)$(bindir)/ssl-cert/% : %
	$(INSTALL) -D -m 755 $< $@

DISTCLEANFILES = ca_rehash check-cert.sh gen-ssl-keys.sh install-cert.sh upgrade-cert.sh
