
EXTRA_DIST = \
    sipxdialog-config.in \
    sipxdialog-config.in.map

install-data-hook :
	@for f in $(EXTRA_DIST); \
	do \
		if ! test -f $(DESTDIR)$(sysconfdir)/sipxpbx/$$f; \
		then \
	    	echo "Installing default $(sysconfdir)/sipxpbx/$$f"; \
			$(INSTALL) -D -m 644 $(srcdir)/$$f $(DESTDIR)$(sysconfdir)/sipxpbx/$$f; \
		else \
	    	echo "Using existing $(sysconfdir)/sipxpbx/$$f"; \
	    fi; \
	done
