include $(top_srcdir)/config/subdir.am

bin_SCRIPTS = \
	${PACKAGE}-config

EXTRA_DIST = \
    $(editscripts_IN) \
    $(bin_SCRIPTS)

$(top_builddir)/BUILDSTAMP:
	${MAKE} -C $(top_builddir) BUILDSTAMP

${PACKAGE}-config: $(top_srcdir)/config/sipX-config.in $(top_builddir)/BUILDSTAMP
	@BuildStamp="$(shell cat $(top_builddir)/BUILDSTAMP)" \
	; ${LocalizeSipXconfig} -e "s/@SIPX_BUILDSTAMP\@/$${BuildStamp}/" \
	    $(top_srcdir)/config/sipX-config.in \
	  > ${PACKAGE}-config
	chmod +x ${PACKAGE}-config

editscripts_IN = \
    sipregistrar.sh.in

.PHONY : editscripts
editscripts :
	@for f in $(editscripts_IN); \
	do \
	  target=`basename $$f .in`; \
	  $(LocalizeSipXconfig) $(srcdir)/$$f > $$target; \
          $(INSTALL) -D -m 755 $$target $(DESTDIR)$(bindir)/$$target; \
	done

install-exec-hook : editscripts
