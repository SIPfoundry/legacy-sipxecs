# where output binaries go 
# Example(to override):
#   make rhel5 DESTDIR=~/my-output
# will put RPMS in ~/my-output/RPM
DESTDIR = $(abspath dist)

ARCH = $(shell uname -p)

%.bld :
	cd $*/build; $(MAKE)

%.cfg :
	cd $*; autoreconf -if
	! test -d $*/build || rm -rf $*/build
	mkdir $*/build
	cd $*/build; \
	  ../configure \
	    --with-distdir=$(DESTDIR) \
	    --cache-file=$(abspath .)/cache-file \
	    --target=$(ARCH)

AC_TARGETS = \
	cppunit \
	cgicc \
	ruby-dbi \
	rubygems \
	ruby-postgres \
	sipx-jasperreports-deps \
	w3c-libwww \
	xerces-c \
	freeswitch \
	rrdtool \
	nsis \
	stunnel

.PHONY: $(AC_TARGETS)
$(AC_TARGETS) : 
	$(MAKE) $@.cfg
	$(MAKE) $@.bld

# required on all platforms
CUSTOM_PACKAGES = cgicc xerces-c ruby-postgres ruby-dbi freeswitch rrdtool nsis sipx-jasperreports-deps

.PHONY: fc8 fc7
fc8 fc7 : $(CUSTOM_PACKAGES) cppunit xerces-c rubygems

.PHONY: f9
f9 : $(CUSTOM_PACKAGES)

.PHONY: rhel5 centos5 rhel4
rhel5 centos5 rhel4 : $(CUSTOM_PACKAGES) w3c-libwww rubygems cppunit

.PHONY: suse11.0
suse11.0 : stunnel w3c-libwww cppunit $(CUSTOM_PACKAGES)
