VERSION=1.5.0
SUN_VERSION=1_5_0
BUILD=10
REL=2

# Combine the SRC and NOSRC to create usable (non-free) rpm
JDK_LIBSRC = $(LIBSRC)/java-sun
SRC_ZIP = $(JDK_LIBSRC)/jdk-$(SUN_VERSION)_$(BUILD)-linux-i586.bin
NOSRC_RPM = $(JDK_LIBSRC)/java-$(VERSION)-sun-$(VERSION).$(BUILD)-$(REL)jpp.nosrc.rpm
RPM_SPEC=java-$(VERSION)-sun.spec
RPMS = \
	$(DEST_RPM)/java-$(VERSION)-sun-$(VERSION).$(BUILD)-$(REL)jpp.i586.rpm \
	$(DEST_RPM)/java-$(VERSION)-sun-alsa-$(VERSION).$(BUILD)-$(REL)jpp.i586.rpm \
	$(DEST_RPM)/java-$(VERSION)-sun-devel-$(VERSION).$(BUILD)-$(REL)jpp.i586.rpm \
	$(DEST_RPM)/java-$(VERSION)-sun-demo-$(VERSION).$(BUILD)-$(REL)jpp.i586.rpm \
	$(DEST_RPM)/java-$(VERSION)-sun-fonts-$(VERSION).$(BUILD)-$(REL)jpp.i586.rpm \
	$(DEST_RPM)/java-$(VERSION)-sun-jdbc-$(VERSION).$(BUILD)-$(REL)jpp.i586.rpm \
	$(DEST_RPM)/java-$(VERSION)-sun-plugin-$(VERSION).$(BUILD)-$(REL)jpp.i586.rpm \
	$(DEST_RPM)/java-$(VERSION)-sun-src-$(VERSION).$(BUILD)-$(REL)jpp.i586.rpm

all : $(RPMS)

$(SRC_ZIP) :
	@echo -e "\nMissing dependency: Download site http://java.sun.com/javase/downloads/index.jsp \n\t$@\n"
	@exit 1

$(NOSRC_RPM) :
	$(DOWNLOAD_FILE) $@ $(JPKG_NONFREE_URL)/SRPMS/`basename $@`

.PHONY: build-rpms
build-rpms : $(NOSRC_RPM) $(SRC_ZIP) 
	rpm -i $(NOSRC_RPM)
	cp $(SRC_ZIP) @RPMBUILD_TOPDIR@/SOURCES
	rpmbuild -ba @RPMBUILD_TOPDIR@/SPECS/$(RPM_SPEC)

$(RPMS) : build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/i586/`basename $@` $@
