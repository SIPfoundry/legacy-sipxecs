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
	java-sun \
	ruby-dbi \
	ruby-postgres \
	sipx-openfire \
	w3c-libwww \
	xerces-c \
	freeswitch \
	rrdtool \
	nsis \
	stunnel \
	openssl \
	libpcap \
	tcpdump \
	asciidoc \
	rubygem-file-tail \
	rsyslog

.PHONY: $(AC_TARGETS)
$(AC_TARGETS) : 
	$(MAKE) $@.cfg
	$(MAKE) $@.bld

.PHONY: erlang_with_no_gui
erlang_with_no_gui:
	sudo yum erase -y  erlang
	$(MAKE) $@.cfg
	$(MAKE) $@.bld
	sudo yum --nogpgcheck -y localinstall  dist/RPM/erlang-R*rpm

.PHONY: rubygems
rubygems:
	sudo yum erase -y  rubygems
	$(MAKE) $@.cfg
	$(MAKE) $@.bld
	sudo yum --nogpgcheck -y localinstall  dist/RPM/rubygems*rpm

.PHONY: rubygem-rake
rubygem-rake:
	sudo yum erase -y  rubygem-rake
	$(MAKE) $@.cfg
	$(MAKE) $@.bld
	sudo yum --nogpgcheck -y localinstall  dist/RPM/rubygem-rake*rpm


# required on all platforms
CUSTOM_PACKAGES = asciidoc ruby-postgres ruby-dbi freeswitch

.PHONY: fc8
fc8 : $(CUSTOM_PACKAGES) nsis cppunit xerces-c rubygems rubygem-rake rubygem-file-tail rrdtool

.PHONY: f10
f10 : $(CUSTOM_PACKAGES) 

.PHONY: f11
f11 : $(CUSTOM_PACKAGES)

.PHONY: f12
f12 : $(CUSTOM_PACKAGES)

.PHONY: rhel5 centos5
rhel5 centos5 : erlang_with_no_gui $(CUSTOM_PACKAGES) rsyslog nsis cppunit xerces-c rubygems rubygem-rake rubygem-file-tail rrdtool stunnel sipx-openfire localrepo

.PHONY: suse11.1
suse11.1 : $(CUSTOM_PACKAGES) nsis libpcap tcpdump openssl stunnel w3c-libwww

.PHONY: localrepo
localrepo:
	 rm -rf `pwd`/dist/RPM/repodata
	 rm -f `pwd`/dist/sipxecs-dependencies-local.repo
	 @echo "creating local repository Meta files"
	 @createrepo `pwd`/dist/RPM/
	 @echo
	 @echo "Creating local repository..."
	 @echo [sipxecs-dependencies-local] > `pwd`/dist/sipxecs-dependencies-local.repo
	 @echo name=sipXecs dependencies local >> `pwd`/dist/sipxecs-dependencies-local.repo
	 @echo baseurl=file://`pwd`/dist/RPM >> `pwd`/dist/sipxecs-dependencies-local.repo
	 @echo enabled=1 >> `pwd`/dist/sipxecs-dependencies-local.repo
	 @echo gpgcheck=0 >> `pwd`/dist/sipxecs-dependencies-local.repo
	 @sudo cp `pwd`/dist/sipxecs-dependencies-local.repo /etc/yum.repos.d/
	 @echo "Added `pwd`/dist/sipxecs-dependencies-local.repo file to repository files in /etc/yum.repos.d"
	 @echo "Your dependencies for sipXecs are now taken from `pwd`/dist/RPM"
