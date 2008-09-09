# Takes the Pingtel ruby-gems package and modifies it so that it becomes
# a rubygems package. rubygems is updateable via yum/yast/etc. while
# ruby-gems is not.

RUBY_GEM_RPM_URL = http://sipxecs.sipfoundry.org/pub/sipXecs/3.6/FC/5/RPM

GEMS_LIBSRC=$(LIBSRC)/ruby-gems
VER = 0.8.11
REL = 2

RPM = ruby-gems-$(VER)-$(REL).noarch.rpm

NEW_RPM = rubygems-$(VER)-$(REL).noarch.rpm
NEW_SPEC = rubygems.spec
WORKING_DIR = var/tmp/ruby-gems_raw		# must be kept in sync with spec file!

ARCH = noarch


all : $(DEST_RPM)/$(NEW_RPM)

# simply download, no need to patch/recompile
$(GEMS_LIBSRC)/$(RPM) :
	$(DOWNLOAD_FILE) $@ $(RUBY_GEM_RPM_URL)/`basename $@`

# download original rpm, install it into a bogus location using rpm
# so that it can perform the transformations of lib->lib64, then
# repackage the file as a binary rpm that can be used for installation.
$(DEST_RPM)/$(NEW_RPM) : $(GEMS_LIBSRC)/$(RPM)
	mkdir -p /var/tmp/ruby-gems_raw
	sudo rpm -U --force --relocate=/=/var/tmp/ruby-gems_raw $(GEMS_LIBSRC)/$(RPM) 
	rpmbuild -ba $(srcdir)/$(NEW_SPEC)
	cp @RPMBUILD_TOPDIR@/RPMS/$(ARCH)/$(NEW_RPM) $@
	rpm -q --info -p $@
