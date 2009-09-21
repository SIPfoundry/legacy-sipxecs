VERSION = 1.1.2
RELEASE = 1

ARCH = noarch

VER_UNDERSCORE = $(subst .,_,$(VERSION))

KRAKEN_URL = http://voxel.dl.sourceforge.net/project/kraken-gateway/kraken-gateway/$(VERSION)
KRAKEN_JAR = kraken.jar

RPMS = \
	$(DEST_RPM)/$(PACKAGE)-$(VERSION)-$(RELEASE).$(ARCH).rpm

SPEC_FILE = $(PACKAGE).spec

all : PRINT_HEADER $(RPMS) PRINT_FOOTER

SRC_TARBALL = $(PACKAGE)-$(VERSION).tar.gz

$(KRAKEN_JAR) :
	$(DOWNLOAD_FILE) $@ $(KRAKEN_URL)/$@

.PHONY: build-rpms
build-rpms : $(KRAKEN_JAR)
	tar czf @RPMBUILD_TOPDIR@/SOURCES/$(SRC_TARBALL) \
	$(KRAKEN_JAR)
	cp $(srcdir)/$(SPEC_FILE) @RPMBUILD_TOPDIR@/SPECS
	rpmbuild -bb \
	  --define 'KRAKEN_VERSION $(VERSION)' \
	  --define 'KRAKEN_RELEASE $(RELEASE)' \
	  --define 'KRAKEN_SOURCE $(SRC_TARBALL)' \
	  --define 'KRAKEN_JAR $(KRAKEN_JAR)' \
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
