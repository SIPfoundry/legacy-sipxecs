# Mostly for centos 5 support
XERCES_C_RPM_URL = $(FC6_EXTRAS_URL)/6/$(RPM_TARGET_ARCH)

XERCES_C_LIBSRC=$(LIBSRC)/xerces-c
VER = 2.7.0
REL = 6

RPMS = \
	xerces-c-$(VER)-$(REL).fc6.$(RPM_TARGET_ARCH).rpm \
	xerces-c-devel-$(VER)-$(REL).fc6.$(RPM_TARGET_ARCH).rpm

all : $(foreach RPM,$(RPMS),$(DEST_RPM)/$(RPM))

# simply download, no need to patch/recompile
$(XERCES_C_LIBSRC)/% :
	$(DOWNLOAD_FILE) $@ $(XERCES_C_RPM_URL)/$*

# simply download, no need to patch/recompile
$(DEST_RPM)/% : $(XERCES_C_LIBSRC)/%
	cp $< $@
