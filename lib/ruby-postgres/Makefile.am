# Mostly for FC5 support because rhe5, fc6 and suse are good
# fc6 rpm installs on fc5 ok
RUBY_PSQL_RPM_URL = $(FC6_EXTRAS_URL)/6/$(RPM_TARGET_ARCH)

RUBY_PSQL_LIBSRC=$(LIBSRC)/ruby-postgres
VER = 0.7.1
REL = 5

RPM = ruby-postgres-$(VER)-$(REL).fc6.$(RPM_TARGET_ARCH).rpm

all : $(DEST_RPM)/$(RPM)

# simply download, no need to patch/recompile
$(RUBY_PSQL_LIBSRC)/$(RPM) :
	$(DOWNLOAD_FILE) $@ $(RUBY_PSQL_RPM_URL)/`basename $@`

# simply download, no need to patch/recompile
$(DEST_RPM)/$(RPM) : $(RUBY_PSQL_LIBSRC)/$(RPM)
	cp $< $@
