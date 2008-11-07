# where output binaries go 
# Example(to override):
#   make rhel5 DESTDIR=~/my-output
# will put RPMS in ~/my-output/RPM
DESTDIR = $(abspath dist)

ARCH = $(shell uname -p)

ifneq (,$(findstring 86,$(ARCH)))
  JAVA_PKG = java-sun
endif 

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
	dnsjava \
	jain-sip \
	jakarta-commons-net \
	java-sun \
	ruby-dbi \
	rubygems \
	ruby-postgres \
	sipx-jasperreports-deps \
	w3c-libwww \
	xerces-c \
	freeswitch \
	rrdtool \
	nsis

.PHONY: $(AC_TARGETS)
$(AC_TARGETS) : 
	$(MAKE) $@.cfg
	$(MAKE) $@.bld

# required on all platforms
CUSTOM_PACKAGES = cgicc xerces-c ruby-postgres ruby-dbi jain-sip conference dnsjava rrdtool nsis sipx-jasperreports-deps $(JAVA_PKG)

.PHONY: fc8 fc7
fc8 fc7 : $(CUSTOM_PACKAGES) cppunit xerces-c rubygems

.PHONY: f9
f9 : $(CUSTOM_PACKAGES)

.PHONY: rhel5 centos5 rhel4
rhel5 centos5 rhel4 : $(CUSTOM_PACKAGES) w3c-libwww jakarta-commons-net rubygems

.PHONY: suse11.0
suse11.0 : w3c-libwww $(CUSTOM_PACKAGES) jakarta-commons-net

.PHONY: conference
conference : freeswitch
