## Process this file with automake to produce Makefile.in

include $(top_srcdir)/config/subdir.am

unconditional_SUBDIRS = . src bin etc doc


editscripts_IN = \
    bin/sipxpark.sh.in

editscripts :
	@for f in $(editscripts_IN); \
	do \
	  target=`basename $$f .in`; \
     $(LocalizeSipXconfig) $(srcdir)/$$f > $$target; \
	  $(INSTALL) -D -m 755 $$target $(DESTDIR)$(bindir)/$$target; \
	done

#EXTRA_DIST = \
#    CONTRIBUTORS
#    sipxpark.spec

SUBDIRS = $(unconditional_SUBDIRS) $(doc_SUBDIRS)
RELEASE = 1

#.PHONY : rpm
# Where rpmbuild will do its work.
#RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')
# Get the revision number
#if USE_BLDNO
#BUILDPARM=--define="buildno 0.$(shell cat $(top_srcdir)/SVN-VERSION)"
#else
#BUILDPARM=--define="buildno $(RELEASE)"
#endif

#rpm : dist
#	rpmbuild -ta $(BUILDPARM) $(PACKAGE)-$(VERSION).tar.gz
#	mv -f $(RPMBUILD_TOPDIR)/SRPMS/$(PACKAGE)-$(VERSION)-*.rpm .
#	mv -f $(RPMBUILD_TOPDIR)/RPMS/*/$(PACKAGE)*-$(VERSION)-*.rpm .
#	md5sum $(PACKAGE)-$(VERSION).tar.gz >$(PACKAGE)-$(VERSION).tar.gz.md5

# RPM Spec file
# Extract the options to ./configure from config.log and propagate them into the .spec file.
#sipxpark.spec : sipxpark.spec.in
#	V="$$( sed -e '/^ *\$$ .*\/configure/!d' -e 's/^.*\/configure *//' config.log )" ; \
#	$(LocalizeSipXconfig) -e "s#@CONFIGURE_OPTIONS@#$$V#" \
#		$(srcdir)/sipxpark.spec.in > sipxpark.spec

# 'rpmbuild -ta' searches root of tarball for pkgname.spec file to build
# RPM from
#dist-hook: sipxpark.spec
#	cp sipxpark.spec $(distdir)
