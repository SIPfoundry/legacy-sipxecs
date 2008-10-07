VERSION = 0.7.1
RELEASE = 2

SRC_DIR = $(srcdir)/src
SRC_TARBALL = ruby-postgres-$(VERSION).tar.gz
SRC_SPEC_FILE = ruby-postgres.spec

RPM = $(DEST_RPM)/ruby-postgres-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm

all : PRINT_HEADER $(RPM) PRINT_FOOTER

$(RPM) : build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/@RPM_TARGET_ARCH@/`basename $@` $@

.PHONY: build-rpms
build-rpms : 
	cp $(SRC_DIR)/$(SRC_TARBALL) @RPMBUILD_TOPDIR@/SOURCES/
	cp $(SRC_SPEC_FILE) @RPMBUILD_TOPDIR@/SPECS/
	rpmbuild -ba @RPMBUILD_TOPDIR@/SPECS/$(SRC_SPEC_FILE)

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

