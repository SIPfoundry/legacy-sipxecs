include $(top_srcdir)/config/subdir.am

SIPX_PROCESS_DEFS = sipstatus

ETC_FILES = \
    status-plugin.xml.in

EXTRA_DIST = \
    $(ETC_FILES) \
    sipstatus.process.xml.in \
    sipstatus-process.xml.in

install-data-hook : $(DESTDIR)$(SIPX_CONFDIR)/process.d/sipstatus.process.xml
	@for f in $(ETC_FILES); \
	do \
		if ! test -f $(DESTDIR)$(sysconfdir)/sipxpbx/$$f; \
		then \
	    	echo "Installing default $(sysconfdir)/sipxpbx/$$f"; \
			$(INSTALL) -D -m 644 $(srcdir)/$$f $(DESTDIR)$(sysconfdir)/sipxpbx/$$f; \
		else \
            diff $(srcdir)/$$f $(DESTDIR)$(sysconfdir)/sipxpbx/$$f > /dev/null 2>&1 \
            || (  echo "Using existing $(srcdir)/$$f - new file is $(DESTDIR)$(sysconfdir)/sipxpbx/$$f.NEW" \
                ; $(INSTALL) -D -m 644 $(srcdir)/$$f $(DESTDIR)$(sysconfdir)/sipxpbx/$$f.NEW; \
                ); \
	    fi; \
	done
