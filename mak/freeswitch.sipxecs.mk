freeswitch_VER = 1.0.7
freeswitch_PACKAGE_REVISION = $(shell cd $(SRC)/$(PROJ); ../config/revision-gen $(freeswitch_VER))

freeswitch_SRPM = freeswitch-$(freeswitch_VER)-$(freeswitch_PACKAGE_REVISION).src.rpm
freeswitch_SPEC = $(SRC)/$(PROJ)/freeswitch.spec
freeswitch_TARBALL = $(BUILDDIR)/$(PROJ)/freeswitch-$(freeswitch_VER).tar.bz2
freeswitch_SOURCES = $(freeswitch_TARBALL) \
	celt-0.10.0.tar.gz \
	flite-1.3.99-latest.tar.gz \
	lame-3.97.tar.gz \
	libshout-2.2.2.tar.gz \
	mpg123-1.13.2.tar.gz \
	openldap-2.4.11.tar.gz \
	pocketsphinx-0.7.tar.gz \
	soundtouch-1.5.0.tar.gz \
	sphinxbase-0.7.tar.gz \
	communicator_semi_6000_20080321.tar.gz \
	libmemcached-0.32.tar.gz \
	json-c-0.9.tar.gz \
	opus-0.9.0.tar.gz

freeswitch_SRPM_DEFS = --define "buildno $(freeswitch_PACKAGE_REVISION)"
freeswitch_RPM_DEFS = --define="buildno $(freeswitch_PACKAGE_REVISION)"

freeswitch.dist : $(SRC)/freeswitch/.git
	test -d $(dir $(freeswitch_TARBALL)) || mkdir -p $(dir $(freeswitch_TARBALL))
	cd $(SRC)/$(PROJ); \
	  git archive --format tar --prefix freeswitch-$(freeswitch_VER)/ HEAD | bzip2 > $(freeswitch_TARBALL)
