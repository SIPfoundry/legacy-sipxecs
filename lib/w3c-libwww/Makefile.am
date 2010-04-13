VERSION = 5.4.1
RELEASE = 0

SRC_DIR = $(srcdir)/src
SRC_TARBALL= $(PACKAGE)-$(VERSION)-$(RELEASE).tar.gz
SRC_SPEC_FILE=$(PACKAGE).spec

RPMS = w3c-libwww-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm \
	w3c-libwww-devel-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm \
	w3c-libwww-apps-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm

all : PRINT_HEADER $(RPMS) PRINT_FOOTER

$(RPMS) : build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/@RPM_TARGET_ARCH@/$@ $(DEST_RPM)/

build-rpms : 
	cp $(SRC_DIR)/$(SRC_TARBALL) @RPMBUILD_TOPDIR@/SOURCES/
	cp $(SRC_DIR)/$(SRC_SPEC_FILE) @RPMBUILD_TOPDIR@/SPECS/
	rpmbuild -ba @RPMBUILD_TOPDIR@/SPECS/$(SRC_SPEC_FILE)
.PHONY: build-rpms

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
