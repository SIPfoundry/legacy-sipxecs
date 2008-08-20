# Mostly for centos 5 support

ARCH = $(shell uname -p)

LIBWWW_TARGET_ARCH = i386

W3C_RPM_URL = $(FC6_EXTRAS_URL)/6/$(LIBWWW_TARGET_ARCH)

W3C_LIBSRC=$(LIBSRC)/w3c-libwww
VER = 5.4.1
REL = 0.4.20060206cvs

RPMS = \
	w3c-libwww-$(VER)-$(REL).fc6.$(LIBWWW_TARGET_ARCH).rpm \
	w3c-libwww-devel-$(VER)-$(REL).fc6.$(LIBWWW_TARGET_ARCH).rpm

all : $(foreach RPM,$(RPMS),$(DEST_RPM)/$(RPM))

# simply download, no need to patch/recompile
$(W3C_LIBSRC)/% :
	$(DOWNLOAD_FILE) $@ $(W3C_RPM_URL)/$*

# simply download, no need to patch/recompile
$(DEST_RPM)/% : $(W3C_LIBSRC)/%
	cp $< $@
