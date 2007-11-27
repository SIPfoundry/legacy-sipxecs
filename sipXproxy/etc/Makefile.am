include $(top_srcdir)/config/subdir.am

SUBDIRS = database

cfgfiles_IN = \
	authrules.xml.in \
	callresolver-config.in \
	sipXproxy-config.in \
	forwardingrules.xml.in

EXTRA_DIST = \
	sipXproxy.process.xml.in \
	sipxcallresolver.process.xml.in \
	sipXproxy-config.in.map \
	$(cfgfiles_IN)

PROCESS_XML_FILES = \
	$(DESTDIR)$(SIPX_PROCDIR)/sipXproxy.process.xml

if ENABLE_CDR
PROCESS_XML_FILES += \
	$(DESTDIR)$(SIPX_PROCDIR)/sipxcallresolver.process.xml
endif

CFG_FILES = $(foreach file,$(cfgfiles_IN),$(DESTDIR)/$(sysconfdir)/sipxpbx/$(file))
install-exec-hook : \
	$(CFG_FILES) \
	upgrade-add-domain \
	upgrade-auth-rules \
	upgrade-forwarding-rules \
	upgrade-auth-plugins \
	upgrade-to-merged-proxy

install-data-hook : \
	$(DESTDIR)$(sysconfdir)/sipxpbx/sipXproxy-config.in.map \
	$(PROCESS_XML_FILES)

$(DESTDIR)$(sysconfdir)/sipxpbx/sipXproxy-config.in.map: sipXproxy-config.in.map
	  echo ""; echo "Installing sipXproxy-config.in.map $@"; \
	  $(INSTALL) -D -m 644 $< $@; 

$(foreach file,$(cfgfiles_IN),$(DESTDIR)/$(sysconfdir)/sipxpbx/$(file)) : $(DESTDIR)/$(sysconfdir)/sipxpbx/% : %.local
	@if ! test -f $@; \
	then \
		echo "Installing default $@"; \
		$(INSTALL) -D -m 644 $< $@; \
	else \
		diff $< $@ > /dev/null 2>&1 \
		|| (  echo "Using existing $@ - new file is $@.NEW" \
		    ; $(INSTALL) -D -m 644 $< $@.NEW; \
			); \
	fi

$(foreach file,$(cfgfiles_IN),$(file).local) : %.local : %
	$(LocalizeSipXconfig) $< > $@

.PHONY: upgrade-auth-rules
upgrade-auth-rules:
	@echo "        Checking/updating schema for $(sysconfdir)/sipxpbx/authrules.xml.in"
	@perl -pi \
	-e 's|<mappings>|<mappings xmlns="http://www.sipfoundry.org/sipX/schema/xml/urlauth-00-00">|' \
	$(DESTDIR)$(sysconfdir)/sipxpbx/authrules.xml.in

.PHONY: upgrade-forwarding-rules
upgrade-forwarding-rules:
	@echo "        Checking/updating schema for $(sysconfdir)/sipxpbx/forwardingrules.xml.in"
	@perl -pi \
	-e 's|<routes>|<routes xmlns="http://www.sipfoundry.org/sipX/schema/xml/forwardingrules-00-00">|' \
	$(DESTDIR)$(sysconfdir)/sipxpbx/forwardingrules.xml.in

.PHONY: upgrade-auth-plugins
upgrade-auth-plugins :
	@echo -n "        Checking for auth plugins..."
	@if ! grep libEnforceAuthRules.so $(DESTDIR)$(sysconfdir)/sipxpbx/sipXproxy-config.in > /dev/null 2>&1 ; \
	then \
		echo -n " 'authrules' added"; \
		( echo "" \
		; echo "SIPX_PROXY_HOOK_LIBRARY.authrules : $(libdir)/authplugins/libEnforceAuthRules.so" \
		; echo "SIPX_PROXY.authrules.RULES        : $(sysconfdir)/sipxpbx/authrules.xml" \
		) >> $(DESTDIR)$(sysconfdir)/sipxpbx/sipXproxy-config.in; \
	else \
		echo -n " 'authrules' ok"; \
	fi; \
	if ! grep libCallerAlias.so $(DESTDIR)$(sysconfdir)/sipxpbx/sipXproxy-config.in > /dev/null 2>&1 ; \
	then \
		echo -n " 'fromalias' added"; \
		( echo "" \
		; echo "SIPX_PROXY_HOOK_LIBRARY.fromalias : $(libdir)/authplugins/libCallerAlias.so" \
		) >> $(DESTDIR)$(sysconfdir)/sipxpbx/sipXproxy-config.in; \
	else \
		echo -n " 'fromalias' ok"; \
	fi
	@echo ""


.PHONY: upgrade-add-domain
upgrade-add-domain :
	@echo -n "        Checking for domain configuration..."
	@if ! grep -e '^SIPX_PROXY_DOMAIN_NAME\>' $(DESTDIR)$(sysconfdir)/sipxpbx/sipXproxy-config.in > /dev/null 2>&1 ; \
	then \
		echo " adding "; \
		( echo "" \
		; echo 'SIPX_PROXY_DOMAIN_NAME : $${SIPXCHANGE_DOMAIN_NAME}' \
		) >> $(DESTDIR)$(sysconfdir)/sipxpbx/sipXproxy-config.in; \
	else \
		echo " ok"; \
	fi

.PHONY: upgrade-to-merged-proxy
upgrade-to-merged-proxy :
	@echo -n "        Checking for deprecated forking and auth proxy leftovers  => "
	@if [ -f $(DESTDIR)$(sysconfdir)/sipxpbx/proxy-config.in ] && [ -f $(DESTDIR)$(sysconfdir)/sipxpbx/authproxy-config.in ]; \
	then \
		$(DESTDIR)$(bindir)/upgrade2mergedproxy --preserve --cleancfgin --cleanlogs --cleanproc --cdir $(DESTDIR)$(sysconfdir)/sipxpbx --ldir $(DESTDIR)$(localstatedir)/log/sipxpbx; \
		echo "  [CLEANED]";\
	else \
		echo "  [NOT FOUND]";\
	fi;


