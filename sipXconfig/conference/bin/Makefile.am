include $(top_srcdir)/config/subdir.am

editscripts_IN = \
    freeswitch.sh.in

EXTRA_DIST = \
   $(editscripts_IN)

# install-exec-hook: $(foreach file,$(editscripts_IN),$(DESTDIR)$(bindir)/$(file))

.PHONY : editscripts
editscripts :
	@for f in $(editscripts_IN); \
	do \
		target=`basename $$f .in`; \
		$(LocalizeSipXconfig) $(srcdir)/$$f > $$target; \
		$(INSTALL) -D -m 755 $$target $(DESTDIR)$(bindir)/$$target; \
	done

install-exec-hook : editscripts

