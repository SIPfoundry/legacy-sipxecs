FS_VER = 1.0.1
FS_REL = 2

SRC_DIR = $(srcdir)/src
SRC_TARBALL = $(SRC_DIR)/freeswitch-$(FS_VER).tar.gz
PATCHED_SRC_TARBALL = sipx-freeswitch-$(FS_VER).tar.gz
SRC_SPEC_FILE=freeswitch.spec

RPM = $(DEST_RPM)/sipx-freeswitch-$(FS_VER)-$(FS_REL).@RPM_TARGET_ARCH@.rpm
SRPM = $(DEST_RPM)/sipx-freeswitch-$(FS_VER)-$(FS_REL).src.rpm
DEVEL_RPM = $(DEST_RPM)/sipx-freeswitch-devel-$(FS_VER)-$(FS_REL).@RPM_TARGET_ARCH@.rpm
AMR_RPM = $(DEST_RPM)/sipx-freeswitch-codec-passthru-amr-$(FS_VER)-$(FS_REL).@RPM_TARGET_ARCH@.rpm
G723_RPM = $(DEST_RPM)/sipx-freeswitch-codec-passthru-g723_1-$(FS_VER)-$(FS_REL).@RPM_TARGET_ARCH@.rpm
G729_RPM = $(DEST_RPM)/sipx-freeswitch-codec-passthru-g729-$(FS_VER)-$(FS_REL).@RPM_TARGET_ARCH@.rpm


RPMS = $(RPM) $(DEVEL_RPM) $(AMR_RPM) $(G723_RPM) $(G729_RPM)

all : PRINT_HEADER $(RPMS) PRINT_FOOTER

.PHONY: build-rpms
build-rpms:
	tar -xzf $(SRC_TARBALL) -C @RPMBUILD_TOPDIR@/SOURCES
	cp $(SRC_DIR)/bootstrap.sh @RPMBUILD_TOPDIR@/SOURCES/freeswitch-$(FS_VER)
	mv -f @RPMBUILD_TOPDIR@/SOURCES/freeswitch-$(FS_VER) @RPMBUILD_TOPDIR@/SOURCES/sipx-freeswitch-$(FS_VER)
	pushd @RPMBUILD_TOPDIR@/SOURCES && tar -czf $(PATCHED_SRC_TARBALL) sipx-freeswitch-$(FS_VER) && rm -rf sipx-freeswitch-$(FS_VER) && popd
	cp $(SRC_SPEC_FILE) @RPMBUILD_TOPDIR@/SPECS/
	QA_RPATHS=0x0003 rpmbuild -ba --target @RPM_TARGET_ARCH@-none-linux @RPMBUILD_TOPDIR@/SPECS/$(SRC_SPEC_FILE)

$(SRPM): build-rpms
	mv @RPMBUILD_TOPDIR@/SRPMS/`basename $@` $@

$(RPMS): build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/@RPM_TARGET_ARCH@/`basename $@` $@

PRINT_HEADER :
	@printf "\n================================================================================\n"
	@printf "Building Package:\n"
	@printf "\tNAME = %s\n" "$(PACKAGE)"
	@printf "\tVERSION = %s\n" "$(FS_VER)"
	@printf "\tRELEASE = %s\n" "$(FS_REL)"
	@printf "\tARCH = %s\n" "@RPM_TARGET_ARCH@"
	@printf "\tStarted building of %s at %s\n" "$(PACKAGE)" "$(shell date)"
	@printf "================================================================================\n"

PRINT_FOOTER :
	@printf "\n================================================================================\n"
	@printf "Finished Building Package %s at %s\n" "$(PACKAGE)" "$(shell date)"
	@printf "\tBinary RPMS can be found under %s\n" "@RPMBUILD_TOPDIR@"
	@printf "================================================================================\n"
