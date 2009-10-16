include $(top_srcdir)/config/subdir.am

bin_SCRIPTS = \
   ${PACKAGE}-config \
   sipxecs-config-ext 

EXTRA_DIST = \
    $(bin_SCRIPTS)

install-exec-hook :

DISTCLEANFILES = Makefile.in config.h.in

$(top_builddir)/BUILDSTAMP:
	${MAKE} -C $(top_builddir) BUILDSTAMP

${PACKAGE}-config: $(top_srcdir)/config/sipX-config.in $(top_builddir)/BUILDSTAMP
	@BuildStamp="$(shell cat $(top_builddir)/BUILDSTAMP)" \
	; ${LocalizeSipXconfig} -e "s/@SIPX_BUILDSTAMP\@/$${BuildStamp}/" \
	    $(top_srcdir)/config/sipX-config.in \
	  > ${PACKAGE}-config
	chmod +x ${PACKAGE}-config

# sipxecs-config-ext is generated from sipxecs-config-ext.in by the
# top-level configure.ac, as it is the only one that knows the names
# of all the components, and so can substitute "subdirs".
