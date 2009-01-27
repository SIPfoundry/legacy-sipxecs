VERSION=1.6.0
SUN_VERSION=6
BUILD=11
REL=1
if IS_AMD64
  SUN_ARCH = x64
  ARCH = @RPM_TARGET_ARCH@
else
  SUN_ARCH = i586
  ARCH = i586
endif

# Combine the SRC and NOSRC to create usable (non-free) rpm
JDK_LIBSRC = $(LIBSRC)/java-sun
SRC_ZIP = $(JDK_LIBSRC)/jdk-$(SUN_VERSION)u$(BUILD)-linux-$(SUN_ARCH).bin
NOSRC_RPM = $(JDK_LIBSRC)/java-$(VERSION)-sun-$(VERSION).$(BUILD)-$(REL)jpp.nosrc.rpm
RPM_SPEC=java-$(VERSION)-sun.spec
RPMS = \
	$(DEST_RPM)/java-$(VERSION)-sun-$(VERSION).$(BUILD)-$(REL)jpp.$(ARCH).rpm \
	$(DEST_RPM)/java-$(VERSION)-sun-alsa-$(VERSION).$(BUILD)-$(REL)jpp.$(ARCH).rpm \
	$(DEST_RPM)/java-$(VERSION)-sun-devel-$(VERSION).$(BUILD)-$(REL)jpp.$(ARCH).rpm \
	$(DEST_RPM)/java-$(VERSION)-sun-demo-$(VERSION).$(BUILD)-$(REL)jpp.$(ARCH).rpm \
	$(DEST_RPM)/java-$(VERSION)-sun-fonts-$(VERSION).$(BUILD)-$(REL)jpp.$(ARCH).rpm \
	$(DEST_RPM)/java-$(VERSION)-sun-jdbc-$(VERSION).$(BUILD)-$(REL)jpp.$(ARCH).rpm \
	$(DEST_RPM)/java-$(VERSION)-sun-src-$(VERSION).$(BUILD)-$(REL)jpp.$(ARCH).rpm

if ! IS_AMD64
  RPMS += $(DEST_RPM)/java-$(VERSION)-sun-plugin-$(VERSION).$(BUILD)-$(REL)jpp.$(ARCH).rpm
endif

all : $(RPMS)

$(SRC_ZIP) :
	@echo -e "\nMissing dependency: \n\t$@\n"
	@echo -e "Download from: http://java.sun.com/javase/downloads or http://java.sun.com/products/archive\n"
	@exit 1

$(NOSRC_RPM) :
	$(DOWNLOAD_FILE) $@ $(JPKG_NONFREE_URL)/SRPMS/`basename $@`

.PHONY: build-rpms
build-rpms : $(NOSRC_RPM) $(SRC_ZIP)
	rpm -i $(NOSRC_RPM)
	cp $(SRC_ZIP) @RPMBUILD_TOPDIR@/SOURCES
	rpmbuild -ba @RPMBUILD_TOPDIR@/SPECS/$(RPM_SPEC)

$(RPMS) : build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/$(ARCH)/`basename $@` $@
