VERSION = 8.4.5
RELEASE = 1

SRC_DIR = $(srcdir)/src
SRC_TARBALL = asciidoc-$(VERSION).tar.bz2
SRPM = $(DEST_SRPM)/asciidoc-$(VERSION)-$(RELEASE).src.rpm
RPMS = \
	$(DEST_RPM)/asciidoc-$(VERSION)-$(RELEASE).noarch.rpm \
	$(DEST_RPM)/asciidoc-examples-$(VERSION)-$(RELEASE).noarch.rpm

SPEC_FILE = asciidoc.spec

all : PRINT_HEADER $(SRPM) $(RPMS) PRINT_FOOTER

$(RPM_SOURCES_DIR)/% : %
	cp $< $@

.PHONY: build-rpms
build-rpms:
	cp $(SRC_DIR)/$(SRC_TARBALL) @RPMBUILD_TOPDIR@/SOURCES
	cp $(SRC_DIR)/*.diff @RPMBUILD_TOPDIR@/SOURCES
	cp $(SRC_DIR)/$(SPEC_FILE) @RPMBUILD_TOPDIR@/SPECS
	QA_RPATHS="\$\[ 0x0001|0x0010 ]" rpmbuild -ba @RPMBUILD_TOPDIR@/SPECS/$(SPEC_FILE)

$(SRPM): build-rpms
	mv @RPMBUILD_TOPDIR@/SRPMS/`basename $@` $@

$(RPMS): build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/noarch/`basename $@` $@

.PHONY: PRINT_HEADER
PRINT_HEADER :
	@printf "\n================================================================================\n"
	@printf "Building Package:\n"
	@printf "\tNAME = %s\n" "$(PACKAGE)"
	@printf "\tVERSION = %s\n" "$(VERSION)"
	@printf "\tRELEASE = %s\n" "$(RELEASE)"
	@printf "\tARCH = %s\n" "noarch"
	@printf "\tStarted building of %s at %s\n" "$(PACKAGE)" "$(shell date)"
	@printf "================================================================================\n"

.PHONY: PRINT_FOOTER
PRINT_FOOTER :
	@printf "\n================================================================================\n"
	@printf "Finished Building Package %s at %s\n" "$(PACKAGE)" "$(shell date)"
	@printf "\tBinary RPMS can be found under %s\n" "@RPMBUILD_TOPDIR@"
	@printf "================================================================================\n"
