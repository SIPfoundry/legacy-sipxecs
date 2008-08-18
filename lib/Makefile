# where output binaries go 
# Example(to override):
#   make rhel5 DESTDIR=~/my-output
# will put RPMS in ~/my-output/RPM
#DESTDIR = $(abspath dist)
DESTDIR=/home/mardy/rpmbuild

ARCH = $(shell uname -p)

ifeq ($(ARCH),ppc)
  JAVA_PKG = java-ibm
else
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
	cgicc \
	cppunit \
	dnsjava \
	jain-sip \
	jakarta-commons-net \
	java-ibm \
	java-sun \
	ruby-dbi \
	ruby-gems \
	ruby-postgres \
	w3c-libwww \
	xerces-c \
	freeswitch \
	rrdtool

.PHONY: $(AC_TARGETS)
$(AC_TARGETS) : 
	$(MAKE) $@.cfg
	$(MAKE) $@.bld

# required on all platforms
CUSTOM_PACKAGES = cgicc xerces-c ruby-dbi ruby-gems ruby-postgres $(JAVA_PKG) rrdtool jain-sip conference dnsjava

.PHONY: fc6
fc6 : $(CUSTOM_PACKAGES) jakarta-commons-net

.PHONY: fc8 fc7
fc8 fc7 : $(CUSTOM_PACKAGES)

.PHONY: rhel5 centos5 rhel4 suse10.2
rhel5 centos5 rhel4 suse10.2 : w3c-libwww $(CUSTOM_PACKAGES) jakarta-commons-net

.PHONY: conference
conference : freeswitch
