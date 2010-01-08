VERSION = 1.0.5
RELEASE = 16184

SRC_DIR = $(srcdir)/src
SRC_TARBALL = sipx-freeswitch-$(VERSION)-$(RELEASE).tar.gz
SPEC_FILE = freeswitch.spec

RPM = $(DEST_RPM)/sipx-freeswitch-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm
SRPM = $(DEST_RPM)/sipx-freeswitch-$(VERSION)-$(RELEASE).src.rpm
DEVEL_RPM = $(DEST_RPM)/sipx-freeswitch-devel-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm
DEBUG_RPM = $(DEST_RPM)/sipx-freeswitch-debuginfo-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm
AMR_RPM = $(DEST_RPM)/sipx-freeswitch-codec-passthru-amr-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm
G723_RPM = $(DEST_RPM)/sipx-freeswitch-codec-passthru-g723_1-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm
G729_RPM = $(DEST_RPM)/sipx-freeswitch-codec-passthru-g729-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm
SCRIPT_RPMS = $(DEST_RPM)/sipx-freeswitch-lua-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm $(DEST_RPM)/sipx-freeswitch-perl-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm $(DEST_RPM)/sipx-freeswitch-spidermonkey-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm
LANG_RPM = $(DEST_RPM)/sipx-freeswitch-lang-en-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm


RPMS = $(RPM) $(DEVEL_RPM) $(SCRIPT_RPMS) $(AMR_RPM) $(G723_RPM) $(G729_RPM) $(LANG_RPM)

all : PRINT_HEADER $(RPMS) PRINT_FOOTER

.PHONY: build-rpms
build-rpms:
	cp $(SRC_DIR)/$(SRC_TARBALL) @RPMBUILD_TOPDIR@/SOURCES
	cp $(SRC_DIR)/$(SPEC_FILE) @RPMBUILD_TOPDIR@/SPECS/
	QA_RPATHS=0x0003 rpmbuild -ba --target @RPM_TARGET_ARCH@-none-linux @RPMBUILD_TOPDIR@/SPECS/$(SPEC_FILE)

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
