include $(top_srcdir)/config/subdir.am

EXTRA_DIST = \
    sipXvxml.process.xml.in \
    mediaserver-config.in.in

install-data-hook : $(DESTDIR)$(SIPX_PROCDIR)/sipXvxml.process.xml
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $(srcdir)/mediaserver-config.in.in > mediaserver-config.in ; \
	if ! test -f $(DESTDIR)$(sysconfdir)/sipxpbx/mediaserver-config.in ; \
	then \
		echo ""; echo "Installing default $(sysconfdir)/sipxpbx/mediaserver-config.in"; \
		$(INSTALL) -D -m 644 mediaserver-config.in $(DESTDIR)$(sysconfdir)/sipxpbx/mediaserver-config.in; \
	else \
        diff mediaserver-config.in $(DESTDIR)$(sysconfdir)/sipxpbx/mediaserver-config.in > /dev/null 2>&1 \
        || (  echo "Using existing - Installing as $(DESTDIR)$(sysconfdir)/sipxpbx/mediaserver-config.in.NEW" \
            ; $(INSTALL) -D -m 644 mediaserver-config.in $(DESTDIR)$(sysconfdir)/sipxpbx/mediaserver-config.in.NEW; \
            ); \
    fi	
