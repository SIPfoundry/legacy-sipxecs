VERSION=1.5.0
BUILD=2.3
REL=3

# Combine the SRC and NOSRC to create usable (non-free) rpm
JDK_LIBSRC = $(LIBSRC)/java-ibm
if IS_PPC
  ARCH = ppc
else
  ARCH = i386
endif
JDK_SRC_ZIP = $(JDK_LIBSRC)/ibm-java2-sdk-5.0-5.1-linux-$(ARCH).tgz
JCOMM_SRC_ZIP = $(JDK_LIBSRC)/ibm-java2-javacomm-5.0-5.1-linux-$(ARCH).tgz
SRC_ZIP = $(JDK_SRC_ZIP) $(JCOMM_SRC_ZIP)
NOSRC_RPM = $(JDK_LIBSRC)/java-$(VERSION)-ibm-$(VERSION).$(BUILD)-$(REL)jpp.nosrc.rpm
RPM_SPEC = java-$(VERSION)-ibm.spec
UNRESTRICTED_SRC = $(JDK_LIBSRC)/unrestrict142.zip
UNRESTRICTED_RPM = $(DEST_RPM)/java-ibm-unrestricted.noarch.rpm
RPMS = \
	$(DEST_RPM)/java-$(VERSION)-ibm-$(VERSION).$(BUILD)-$(REL)jpp.$(ARCH).rpm \
	$(DEST_RPM)/java-$(VERSION)-ibm-devel-$(VERSION).$(BUILD)-$(REL)jpp.$(ARCH).rpm \
	$(DEST_RPM)/java-$(VERSION)-ibm-demo-$(VERSION).$(BUILD)-$(REL)jpp.$(ARCH).rpm \
	$(DEST_RPM)/java-$(VERSION)-ibm-jdbc-$(VERSION).$(BUILD)-$(REL)jpp.$(ARCH).rpm \
	$(DEST_RPM)/java-$(VERSION)-ibm-javacomm-$(VERSION).$(BUILD)-$(REL)jpp.$(ARCH).rpm \
	$(DEST_RPM)/java-$(VERSION)-ibm-plugin-$(VERSION).$(BUILD)-$(REL)jpp.$(ARCH).rpm \
	$(DEST_RPM)/java-$(VERSION)-ibm-src-$(VERSION).$(BUILD)-$(REL)jpp.$(ARCH).rpm

PHONY_SOURCES = \
	ibm-java2-sdk-50-linux-x86_64.tgz \
	ibm-java2-sdk-50-linux-s390.tgz \
	ibm-java2-sdk-50-linux-s390x.tgz \
	ibm-java2-sdk-50-linux-ppc64.tgz \
	ibm-java2-sdk-50-linux-i386.tgz \
	ibm-java2-javacomm-50-linux-x86_64.tgz \
	ibm-java2-javacomm-50-linux-ppc64.tgz \
	ibm-java2-javacomm-50-linux-i386.tgz \
	ibm-java2-sdk-50-linux-ppc.tgz \
	ibm-java2-javacomm-50-linux-ppc.tgz

all :  $(RPMS) $(UNRESTRICTED_RPM)

$(UNRESTRICTED_SRC) $(SRC_ZIP) :
	@echo -e "\nMissing dependency: Download this from www.ibm.com \n\t$@\n"
	@exit 1

$(NOSRC_RPM) :
	$(DOWNLOAD_FILE) $@ $(JPKG_NONFREE_URL)/SRPMS/`basename $@`

# ibm spec file complains if these files do not exist even though we're
# not asking it to build these targets.
.PHONY:phony-sources
phony-sources :
	touch $(foreach F,$(PHONY_SOURCES),@RPMBUILD_TOPDIR@/SOURCES/$F)

.PHONY:build-rpms
build-rpms: $(NOSRC_RPM) $(SRC_ZIP) phony-sources
	rpm -i $(NOSRC_RPM)
	cp $(JDK_SRC_ZIP) @RPMBUILD_TOPDIR@/SOURCES/ibm-java2-sdk-50-linux-$(ARCH).tgz
	cp $(JCOMM_SRC_ZIP) @RPMBUILD_TOPDIR@/SOURCES/ibm-java2-javacomm-50-linux-$(ARCH).tgz
	rpmbuild -ba @RPMBUILD_TOPDIR@/SPECS/$(RPM_SPEC)

$(RPMS) : build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/$(ARCH)/`basename $@` $@

$(UNRESTRICTED_RPM) : $(UNRESTRICTED_SRC) $(srcdir)/java-ibm-unrestricted.spec
	cp $(UNRESTRICTED_SRC) @RPMBUILD_TOPDIR@/SOURCES
	rpmbuild --target noarch --quiet -ba $(srcdir)/java-ibm-unrestricted.spec
	mv @RPMBUILD_TOPDIR@/RPMS/noarch/`basename $@` $@
