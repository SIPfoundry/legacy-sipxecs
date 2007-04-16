VERSION=1.5.0
IBM_VERSION=1_5_0
BUILD=2.3
REL=1

# Combine the SRC and NOSRC to create usable (non-free) rpm
JDK_LIBSRC = $(LIBSRC)/java-ibm
SRC_ZIP = $(JDK_LIBSRC)/ibm-java2-sdk-50-linux-i386.tgz
NOSRC_RPM = $(JDK_LIBSRC)/java-$(VERSION)-ibm-$(VERSION).$(BUILD)-$(REL)jpp.nosrc.rpm
RPM_SPEC = java-$(VERSION)-ibm.spec
UNRESTRICTED_SRC = $(JDK_LIBSRC)/unrestrict142.zip
UNRESTRICTED_RPM = $(DEST_RPM)/java-ibm-unrestricted.noarch.rpm
RPMS = \
	$(DEST_RPM)/java-$(VERSION)-ibm-$(VERSION).$(BUILD)-$(REL)jpp.i586.rpm \
	$(DEST_RPM)/java-$(VERSION)-ibm-alsa-$(VERSION).$(BUILD)-$(REL)jpp.i586.rpm \
	$(DEST_RPM)/java-$(VERSION)-ibm-devel-$(VERSION).$(BUILD)-$(REL)jpp.i586.rpm \
	$(DEST_RPM)/java-$(VERSION)-ibm-demo-$(VERSION).$(BUILD)-$(REL)jpp.i586.rpm \
	$(DEST_RPM)/java-$(VERSION)-ibm-fonts-$(VERSION).$(BUILD)-$(REL)jpp.i586.rpm \
	$(DEST_RPM)/java-$(VERSION)-ibm-jdbc-$(VERSION).$(BUILD)-$(REL)jpp.i586.rpm \
	$(DEST_RPM)/java-$(VERSION)-ibm-plugin-$(VERSION).$(BUILD)-$(REL)jpp.i586.rpm \
	$(DEST_RPM)/java-$(VERSION)-ibm-src-$(VERSION).$(BUILD)-$(REL)jpp.i586.rpm

all :  $(RPMS) $(UNRESTRICTED_RPM)

$(UNRESTRICTED_SRC) $(SRC_ZIP) :
	@echo -e "\nMissing dependency: Download this from www.ibm.com \n\t$@\n"
	@exit 1

$(NOSRC_RPM) :
	$(DOWNLOAD_FILE) $@ $(JPKG_NONFREE_URL)/SRPMS/`basename $@`

.PHONY:build-rpms
build-rpms: $(NOSRC_RPM) $(SRC_ZIP)
	rpm -i $(NOSRC_RPM)
	cp $(SRC_ZIP) @RPMBUILD_TOPDIR@/SOURCES
	rpmbuild -ba @RPMBUILD_TOPDIR@/SPECS/$(RPM_SPEC)

$(RPMS) : build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/i586/`basename $@` $@

$(UNRESTRICTED_RPM) : $(UNRESTRICTED_SRC) $(srcdir)/java-ibm-unrestricted.spec
	cp $(UNRESTRICTED_SRC) @RPMBUILD_TOPDIR@/SOURCES
	rpmbuild --target noarch --quiet -ba $(srcdir)/java-ibm-unrestricted.spec
	mv @RPMBUILD_TOPDIR@/RPMS/noarch/`basename $@` $@
