include $(top_srcdir)/config/subdir.am

editscripts_IN = \
    sipxacccode.sh.in

.PHONY : editscripts
editscripts :
	@for f in $(editscripts_IN); \
	do \
	  target=`basename $$f .in`; \
     $(LocalizeSipXconfig) $(srcdir)/$$f > $$target; \
	  $(INSTALL) -D -m 755 $$target $(DESTDIR)$(bindir)/$$target; \
	done

EXTRA_DIST = \
    $(editscripts_IN)

install-exec-hook : editscripts
