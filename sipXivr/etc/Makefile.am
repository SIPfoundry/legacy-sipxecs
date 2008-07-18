include $(top_srcdir)/config/subdir.am

SIPX_PROCESS_DEFS = \
	sipxivr

ETC_FILES = 

EXTRA_DIST = \
    $(ETC_FILES) \
    sipxivr.process.xml.in \
    sipxivr-process.xml.in

install-data-hook : $(DESTDIR)$(SIPX_CONFDIR)/process.d/sipxivr.process.xml
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
