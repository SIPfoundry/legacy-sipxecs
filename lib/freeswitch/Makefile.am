VERSION = 1.0.7
GIT_RELEASE = 89d99a9
RELEASE = $(GIT_RELEASE)

SRC_DIR = $(srcdir)/src
GIT_URL = git://git.freeswitch.org/freeswitch.git
SRC_TARBALL = sipx-freeswitch-$(VERSION)-$(GIT_RELEASE).tar.bz2
SPEC_FILE = sipx-freeswitch.spec

RPM = $(DEST_RPM)/sipx-freeswitch-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm
SRPM = $(DEST_RPM)/sipx-freeswitch-$(VERSION)-$(RELEASE).src.rpm
DEVEL_RPM = $(DEST_RPM)/sipx-freeswitch-devel-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm
DEBUG_RPM = $(DEST_RPM)/sipx-freeswitch-debuginfo-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm
AMR_RPM = $(DEST_RPM)/sipx-freeswitch-codec-passthru-amr-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm
AMRWB_RPM = $(DEST_RPM)/sipx-freeswitch-codec-passthru-amrwb-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm
G723_RPM = $(DEST_RPM)/sipx-freeswitch-codec-passthru-g723_1-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm
G729_RPM = $(DEST_RPM)/sipx-freeswitch-codec-passthru-g729-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm
EN_RPM = $(DEST_RPM)/sipx-freeswitch-lang-en-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm
DE_RPM = $(DEST_RPM)/sipx-freeswitch-lang-de-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm
LUA_RPM = $(DEST_RPM)/sipx-freeswitch-lua-$(VERSION)-$(RELEASE).@RPM_TARGET_ARCH@.rpm
RPMS = $(RPM) $(DEVEL_RPM) $(AMR_RPM) $(AMRWB_RPM) $(G723_RPM) $(G729_RPM) $(EN_RPM) $(DE_RPM) $(LUA_RPM)

all : PRINT_HEADER $(RPMS) PRINT_FOOTER

.PHONY: build-rpms
build-rpms:
	pushd @RPMBUILD_TOPDIR@/SOURCES ; \
	git  clone $(GIT_URL) sipx-freeswitch-$(VERSION) ; \
	cd sipx-freeswitch-$(VERSION) ; \
	git archive --format tar --prefix sipx-freeswitch-$(VERSION)/ $(GIT_RELEASE)| bzip2 > ../$(SRC_TARBALL) ; \
	cd .. ; \
	rm -rf sipx-freeswitch-$(VERSION) ; \
	popd
	cp $(srcdir)/$(SPEC_FILE) @RPMBUILD_TOPDIR@/SPECS/
	QA_RPATHS=0x0003 rpmbuild -ba \
	  --define 'VERSION $(VERSION)' \
	  --define 'RELEASE $(RELEASE)' \
	  --define 'SOURCE $(SRC_TARBALL)' \
	  --target @RPM_TARGET_ARCH@-none-linux @RPMBUILD_TOPDIR@/SPECS/$(SPEC_FILE)

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
