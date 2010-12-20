include $(top_srcdir)/config/subdir.am

EXTRA_DIST = \
    $(editscripts_IN)

editscripts_IN = \
    sipregistrar.sh.in

install-exec-hook : editscripts

.PHONY : editscripts
editscripts :
	@for f in $(editscripts_IN); \
	do \
	  target=`basename $$f .in`; \
	  $(LocalizeSipXconfig) $(srcdir)/$$f > $$target; \
          $(INSTALL) -D -m 755 $$target $(DESTDIR)$(bindir)/$$target; \
	done

