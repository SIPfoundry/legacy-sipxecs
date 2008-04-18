VERSION=1.5.0
BUILD=7
REL=1

# Combine the SRC and NOSRC to create usable (non-free) rpm
JDK_LIBSRC = $(LIBSRC)/java-ibm
if IS_PPC
  ARCH = ppc
else
  ARCH = i386
endif
JDK_JPKG_ZIP = $(JDK_LIBSRC)/pxp32dev-20080315-jpackage.tar.gz
UNRESTRICTED_SRC = $(JDK_LIBSRC)/unrestrict142.zip
UNRESTRICTED_RPM = $(DEST_RPM)/java-ibm-unrestricted.noarch.rpm
RPMS = \
	$(DEST_RPM)/java-$(VERSION)-ibm-$(VERSION).$(BUILD)-$(REL)jpp.$(ARCH).rpm \
	$(DEST_RPM)/java-$(VERSION)-ibm-alsa-$(VERSION).$(BUILD)-$(REL)jpp.$(ARCH).rpm \
	$(DEST_RPM)/java-$(VERSION)-ibm-devel-$(VERSION).$(BUILD)-$(REL)jpp.$(ARCH).rpm \
	$(DEST_RPM)/java-$(VERSION)-ibm-demo-$(VERSION).$(BUILD)-$(REL)jpp.$(ARCH).rpm \
	$(DEST_RPM)/java-$(VERSION)-ibm-fonts-$(VERSION).$(BUILD)-$(REL)jpp.$(ARCH).rpm \
	$(DEST_RPM)/java-$(VERSION)-ibm-src-$(VERSION).$(BUILD)-$(REL)jpp.$(ARCH).rpm

all :  $(RPMS) $(UNRESTRICTED_RPM)

$(UNRESTRICTED_SRC) $(SRC_ZIP) :
	@echo -e "\nMissing dependency: Download this from www.ibm.com \n\t$@\n"
	@exit 1

$(RPMS) :
	tar -xzf $(JDK_JPKG_ZIP) -C $(DEST_RPM)

$(UNRESTRICTED_RPM) : $(UNRESTRICTED_SRC) $(srcdir)/java-ibm-unrestricted.spec
	cp $(UNRESTRICTED_SRC) @RPMBUILD_TOPDIR@/SOURCES
	rpmbuild --target noarch --quiet -ba $(srcdir)/java-ibm-unrestricted.spec
	mv @RPMBUILD_TOPDIR@/RPMS/noarch/`basename $@` $@
