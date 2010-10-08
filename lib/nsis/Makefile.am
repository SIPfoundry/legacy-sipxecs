VERSION = 2.39
RELEASE = 1

SRC_DIR = $(srcdir)/src
SRC_TARBALL = nsis-$(VERSION)-src.tar.bz2
DATA_TARBALL = nsis-$(VERSION).zip
SRC_SPEC_FILE=nsis.spec
DATA_SPEC_FILE=nsis-data.spec

NSIS_RPM = $(DEST_RPM)/nsis-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm
NSIS_DATA_RPM = $(DEST_RPM)/nsis-data-$(VERSION)-$(RELEASE).noarch.rpm
RPMS = $(NSIS_RPM) $(NSIS_DATA_RPM)

PATCH = nsis.patch

all : PRINT_HEADER $(RPMS) PRINT_FOOTER

.PHONY: build-nsis-rpm
build-nsis-rpm: $(SRC_DIR)/$(SRC_TARBALL) $(SRC_DIR)/$(PATCH)
	cp $(SRC_DIR)/$(SRC_TARBALL) @RPMBUILD_TOPDIR@/SOURCES
	cp $(SRC_DIR)/$(PATCH) @RPMBUILD_TOPDIR@/SOURCES
	cp $(SRC_DIR)/$(SRC_SPEC_FILE) @RPMBUILD_TOPDIR@/SPECS/
	QA_RPATHS="\$\[ 0x0001|0x0010 ]" rpmbuild -ba --target @RPM_TARGET_ARCH@-none-linux @RPMBUILD_TOPDIR@/SPECS/$(SRC_SPEC_FILE)

.PHONY: build-nsis-data-rpm
build-nsis-data-rpm: $(SRC_DIR)/$(DATA_TARBALL)
	cp $(SRC_DIR)/$(DATA_TARBALL) @RPMBUILD_TOPDIR@/SOURCES
	cp $(SRC_DIR)/$(DATA_SPEC_FILE) @RPMBUILD_TOPDIR@/SPECS/
	rpmbuild -ba --target noarch-none-linux @RPMBUILD_TOPDIR@/SPECS/$(DATA_SPEC_FILE)

$(NSIS_RPM): build-nsis-rpm
	mv @RPMBUILD_TOPDIR@/RPMS/@RPM_TARGET_ARCH@/`basename $@` $@

$(NSIS_DATA_RPM): build-nsis-data-rpm
	mv @RPMBUILD_TOPDIR@/RPMS/noarch/`basename $@` $@

$(RPMS): $(NSIS_RPM) $(NSIS_DATA_RPM)

PRINT_HEADER :
	@printf "\n================================================================================\n"
	@printf "Building Package:\n"
	@printf "\tNAME = %s\n" "$(PACKAGE)"
	@printf "\tVERSION = %s\n" "$(VERSION)"
	@printf "\tRELEASE = %s\n" "$(RELEASE)"
	@printf "\tARCH = %s\n" "@RPM_TARGET_ARCH@"
	@printf "\tStarted building of %s at %s\n" "$(PACKAGE)" "$(shell date)"
	@printf "================================================================================\n"

PRINT_FOOTER :
	@printf "\n================================================================================\n"
	@printf "Finished Building Package %s at %s\n" "$(PACKAGE)" "$(shell date)"
	@printf "\tBinary RPMS can be found under %s\n" "@RPMBUILD_TOPDIR@"
	@printf "================================================================================\n"
