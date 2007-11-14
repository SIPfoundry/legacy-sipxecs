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
	xerces-c

.PHONY: $(AC_TARGETS)
$(AC_TARGETS) : 
	$(MAKE) $@.cfg
	$(MAKE) $@.bld

# required on all platforms
CUSTOM_PACKAGES = cgicc ruby-dbi $(JAVA_PKG)

.PHONY: fc6 fc7 fc8
fc7 fc8 fc6 : $(CUSTOM_PACKAGES)

.PHONY: fc5
fc5 : ruby-gems $(CUSTOM_PACKAGES)

.PHONY: rhel5 centos5
centos5 rhel5 : w3c-libwww xerces-c ruby-gems ruby-postgres $(CUSTOM_PACKAGES)
