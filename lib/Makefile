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
	java-sun \
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
CUSTOM_PACKAGES = cgicc ruby-postgres ruby-dbi freeswitch nsis sipx-jasperreports-deps

.PHONY: fc8
fc8 : $(CUSTOM_PACKAGES) cppunit xerces-c rubygems rrdtool

.PHONY: f10
f10 : $(CUSTOM_PACKAGES) 

.PHONY: rhel5 centos5
rhel5 centos5 : $(CUSTOM_PACKAGES) cppunit xerces-c w3c-libwww rubygems rrdtool

.PHONY: suse11.1
suse11.1 : $(CUSTOM_PACKAGES) stunnel w3c-libwww
