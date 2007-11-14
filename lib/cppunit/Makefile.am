# Mostly for centos 5 support
CPPUNIT_RPM_URL = $(FC6_EXTRAS_URL)/6/$(RPM_TARGET_ARCH)

CPPUNIT_LIBSRC=$(LIBSRC)/cppunit
VER = 1.12.0
REL = 2

RPMS = \
	cppunit-$(VER)-$(REL).fc6.$(RPM_TARGET_ARCH).rpm \
	cppunit-devel-$(VER)-$(REL).fc6.$(RPM_TARGET_ARCH).rpm

all : $(foreach RPM,$(RPMS),$(DEST_RPM)/$(RPM))

# simply download, no need to patch/recompile
$(CPPUNIT_LIBSRC)/% :
	$(DOWNLOAD_FILE) $@ $(CPPUNIT_RPM_URL)/$*

# simply download, no need to patch/recompile
$(DEST_RPM)/% : $(CPPUNIT_LIBSRC)/%
	cp $< $@
