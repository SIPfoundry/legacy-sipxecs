VERSION = 3.6.4
RELEASE = 2

ARCH = noarch

VER_UNDERSCORE = $(subst .,_,$(VERSION))

JIVE_OPENFIRE_URL = http://www.igniterealtime.org/downloadServlet?filename=openfire
BINARY_TARBALL = openfire_$(VER_UNDERSCORE).tar.gz

RPMS = \
	$(DEST_RPM)/$(PACKAGE)-$(VERSION)-$(RELEASE).$(ARCH).rpm

SPEC_FILE = $(PACKAGE).spec

all : PRINT_HEADER $(RPMS) PRINT_FOOTER

SRC_TARBALL = $(PACKAGE)-$(VERSION).tar.gz

$(BINARY_TARBALL) :
	$(DOWNLOAD_FILE) $@ $(JIVE_OPENFIRE_URL)/$@

.PHONY: build-rpms
build-rpms : $(BINARY_TARBALL)
	tar czf @RPMBUILD_TOPDIR@/SOURCES/$(SRC_TARBALL) \
	$(BINARY_TARBALL)
	cp $(srcdir)/$(SPEC_FILE) @RPMBUILD_TOPDIR@/SPECS
	rpmbuild -bb \
	  --define 'OPENFIRE_VERSION $(VERSION)' \
	  --define 'OPENFIRE_RELEASE $(RELEASE)' \
	  --define 'OPENFIRE_SOURCE $(SRC_TARBALL)' \
	  --define 'OPENFIRE_BINARY $(BINARY_TARBALL)' \
	  --define 'SIPXPBXUSER $(SIPXPBXUSER)' \
	  --define 'SIPXPBXGROUP $(SIPXPBXGROUP)' \
	  --target $(ARCH)-none-linux @RPMBUILD_TOPDIR@/SPECS/$(SPEC_FILE)

$(RPMS) : build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/$(ARCH)/`basename $@` $@

.PHONY: PRINT_HEADER
PRINT_HEADER :
	@printf "\n================================================================================\n"
	@printf "Building Package:\n"
	@printf "\tNAME = %s\n" "$(PACKAGE)"
	@printf "\tVERSION = %s\n" "$(VERSION)"
	@printf "\tRELEASE = %s\n" "$(RELEASE)"
	@printf "\tARCH = %s\n" "$(ARCH)"
	@printf "\tStarted building of %s at %s\n" "$(PACKAGE)" "$(shell date)"
	@printf "================================================================================\n"

.PHONY: PRINT_FOOTER
PRINT_FOOTER :
	@printf "\n================================================================================\n"
	@printf "Finished Building Package %s at %s\n" "$(PACKAGE)" "$(shell date)"
	@printf "\tBinary RPMS can be found under %s\n" "@RPMBUILD_TOPDIR@"
	@printf "================================================================================\n"
