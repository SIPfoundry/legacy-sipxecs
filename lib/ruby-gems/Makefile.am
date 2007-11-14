# Mostly for FC5 support because rhe5, fc6 and suse are good
# fc6 rpm installs on fc5 ok
RUBY_GEM_RPM_URL = $(FC6_EXTRAS_URL)/6/$(RPM_TARGET_ARCH)

GEMS_LIBSRC=$(LIBSRC)/ruby-gems
VER = 0.9.2
REL = 1

RPM = rubygems-$(VER)-$(REL).fc6.noarch.rpm

all : $(DEST_RPM)/$(RPM)

# simply download, no need to patch/recompile
$(GEMS_LIBSRC)/$(RPM) :
	$(DOWNLOAD_FILE) $@ $(RUBY_GEM_RPM_URL)/`basename $@`

# simply download, no need to patch/recompile
$(DEST_RPM)/$(RPM) : $(GEMS_LIBSRC)/$(RPM)
	cp $< $@
