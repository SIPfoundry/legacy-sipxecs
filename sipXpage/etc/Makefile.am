include $(top_srcdir)/config/subdir.am

SIPX_PROCESS_DEFS = \
	sipxpage

ETC_FILES = \
    sipxpage.properties.in

EXTRA_DIST = \
    $(ETC_FILES) \
    sipxpage.process.xml.in \
    sipxpage-process.xml.in

install-data-hook : $(DESTDIR)$(SIPX_CONFDIR)/process.d/sipxpage.process.xml
	@for f in $(ETC_FILES); \
	do \
		if ! test -f $(DESTDIR)$(sysconfdir)/sipxpbx/$$f; \
		then \
	    	echo "Installing default $(sysconfdir)/sipxpbx/$$f"; \
			$(INSTALL) -D -m 644 $(srcdir)/$$f $(DESTDIR)$(sysconfdir)/sipxpbx/$$f; \
		else \
	    	echo "Using existing $(sysconfdir)/sipxpbx/$$f"; \
	    fi; \
	done
