## Process this file with automake to produce Makefile.in
include config/project.am

RPMS = $(DEFAULT_RPMS) $(DEBUG_RPM) \
	$(DEST_RPM)/sipxcommserverlib-doc-$(SIPX_VERSION).$(RPM_TARGET_ARCH).rpm

EXTRA_DIST = \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/svn-version \
    config/Doxyfile.in \
    CONTRIBUTORS \
    SVN-VERSION \
    BUILDSTAMP \
    sipxcommserverlib.spec

include config/conditional_doc.am

SUBDIRS = \
	. \
	src \
	include \
	etc \
	meta \
	bin \
	$(doc_SUBDIRS)

# The final commands edit an existing subscription.xml file to add the
# "component", "accept", and "version" columns, if they are not already
# present.  In addition, they remove cached IMDB shared memory segments.
# Those commands need to be retained until all developers are
# upgraded, which will be at the end of the 3.9 development period.
install-data-hook: install-common-dirs upgrade-subscription-db

COMMON_DIRS = \
	$(sysconfdir)/sipxpbx \
	$(datadir)/sipxecs \
	$(libexecdir)/sipXecs \
	$(libexecdir)/sipXecs/setup.d \
	$(localstatedir)/log/sipxpbx \
	$(localstatedir)/run/sipxpbx \
	$(localstatedir)/sipxdata/tmp \
	$(localstatedir)/sipxdata/sipdb

.PHONY: install-common-dirs
install-common-dirs: $(foreach dir,$(COMMON_DIRS),$(DESTDIR)$(dir))

$(foreach dir,$(COMMON_DIRS),$(DESTDIR)$(dir)) : % :
	$(INSTALL) -d -m 755 $@

.PHONY: upgrade-subscription-db
upgrade-subscription-db:
	SUBSCRIPTION=$(DESTDIR)$(localstatedir)/sipxdata/sipdb/subscription.xml ; \
	if [ -f $$SUBSCRIPTION ] && \
	   ! grep '<component>' $$SUBSCRIPTION >/dev/null ; then \
	        perl -i -pe 's%<item>%<item><component>status</component><accept>application/simple-message-summary</accept><version>0</version>%' $$SUBSCRIPTION ; \
	        rm -f $(DESTDIR)$(localstatedir)/sipxdata/tmp/imdb.* ; \
	fi

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
