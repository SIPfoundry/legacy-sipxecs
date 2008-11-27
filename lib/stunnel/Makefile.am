PACKAGE = stunnel
VERSION = 4.26
RELEASE = 1

SRC_DIR = $(srcdir)/src
SRC_TARBALL = $(PACKAGE)-$(VERSION).tar.gz
SRC_SPEC_FILE=$(PACKAGE).spec

SRPM = $(DEST_SRPM)/$(PACKAGE)-$(VERSION)-$(RELEASE).src.rpm
RPMS = $(DEST_RPM)/$(PACKAGE)-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm

all : PRINT_HEADER $(SRPM) $(RPMS) PRINT_FOOTER

.PHONY: build-rpms
build-rpms: $(SRC)
	cp $(SRC_DIR)/$(SRC_TARBALL) @RPMBUILD_TOPDIR@/SOURCES
	cp $(SRC_DIR)/$(SRC_SPEC_FILE) @RPMBUILD_TOPDIR@/SPECS/
	rpmbuild -ba --target @RPM_TARGET_ARCH@-none-linux @RPMBUILD_TOPDIR@/SPECS/$(SRC_SPEC_FILE)

$(SRPM): build-rpms
	mv @RPMBUILD_TOPDIR@/SRPMS/`basename $@` $@

$(RPMS): build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/@RPM_TARGET_ARCH@/`basename $@` $@

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

