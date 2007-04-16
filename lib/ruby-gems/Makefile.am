# Original source dlutter stopped making rpms available so
# download from ourselves.  This is only needed on CentOS 4 and RHEL 4
# distros and will disappear when lasted distro stops building them
RUBY_GEM_RPM_URL = http://www.sipfoundry.org/pub/sipX/3.6/FC/5/RPM

GEMS_LIBSRC=$(LIBSRC)/ruby-gems
VER = 0.8.11
REL = 2

RPM = ruby-gems-$(VER)-$(REL).noarch.rpm

all : $(DEST_RPM)/$(RPM)

# simply download, no need to patch/recompile
$(GEMS_LIBSRC)/$(RPM) :
	$(DOWNLOAD_FILE) $@ $(RUBY_GEM_RPM_URL)/`basename $@`

# simply download, no need to patch/recompile
$(DEST_RPM)/$(RPM) : $(GEMS_LIBSRC)/$(RPM)
	cp $< $@
