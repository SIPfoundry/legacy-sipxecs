freeswitch_VER = 1.4.6
freeswitch_TAG = 1.4.4
freeswitch_PACKAGE_REVISION = $(shell cd $(SRC)/$(PROJ); ../config/revision-gen $(freeswitch_TAG))
freeswitch_SRPM = freeswitch-$(freeswitch_VER)-$(freeswitch_PACKAGE_REVISION).src.rpm
freeswitch_SPEC = $(SRC)/$(PROJ)/freeswitch.spec
freeswitch_TARBALL = $(BUILDDIR)/$(PROJ)/freeswitch-$(freeswitch_VER).tar.bz2
freeswitch_SOURCES = $(freeswitch_TARBALL) \
	v8-3.24.14.tar.bz2 \
	opus-1.1.tar.gz \
	soundtouch-1.7.1.tar.gz \
	lame-3.98.4.tar.gz \
	flite-1.5.4-current.tar.bz2 \
	pcre-8.34.tar.gz

freeswitch_SRPM_DEFS = \
	--define "BUILD_NUMBER $(freeswitch_PACKAGE_REVISION)" \
	--define "VERSION_NUMBER $(freeswitch_VER)"

freeswitch_RPM_DEFS = \
	--define="BUILD_NUMBER $(freeswitch_PACKAGE_REVISION)" \
	--define "VERSION_NUMBER $(freeswitch_VER)"

freeswitch.dist : $(SRC)/freeswitch/.git
	test -d $(dir $(freeswitch_TARBALL)) || mkdir -p $(dir $(freeswitch_TARBALL))
	cd $(SRC)/$(PROJ); \
	  git archive --format tar --prefix freeswitch-$(freeswitch_VER)/ HEAD | bzip2 > $(freeswitch_TARBALL)
