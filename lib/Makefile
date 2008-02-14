# where output binaries go 
# Example(to override):
#   make rhel5 DESTDIR=~/my-output
# will put RPMS in ~/my-output/RPM
DESTDIR = $(abspath dist)

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
	java-ibm \
	java-sun \
	ruby-dbi \
	ruby-gems \
	ruby-postgres \
	w3c-libwww \
	xerces-c \
	rrdtool

.PHONY: $(AC_TARGETS)
$(AC_TARGETS) : 
	$(MAKE) $@.cfg
	$(MAKE) $@.bld

# required on all platforms
CUSTOM_PACKAGES = cgicc xerces-c ruby-dbi $(JAVA_PKG) rrdtool

.PHONY: fc8 fc7 fc6
fc8 fc7 fc6 : $(CUSTOM_PACKAGES)

.PHONY: rhel5 centos5 rhel4
rhel5 centos5 rhel4 : w3c-libwww ruby-gems ruby-postgres $(CUSTOM_PACKAGES)
