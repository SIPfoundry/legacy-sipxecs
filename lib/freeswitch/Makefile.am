FS_VER = 1.0
FS_REL = rc3-1
FS_URL=http://files.freeswitch.org

SRC = $(LIBSRC)/freeswitch-1.0.rc3.tar.gz
PATCHED_SRC = $(LIBSRC)/sipX-freeswitch-1.0.rc3.tar.gz

RPM = $(DEST_RPM)/sipX-freeswitch-$(FS_VER).$(FS_REL).@RPM_TARGET_ARCH@.rpm
SRPM = $(DEST_RPM)/sipX-freeswitch-$(FS_VER).$(FS_REL).src.rpm
DEVEL_RPM = $(DEST_RPM)/sipX-freeswitch-devel-$(FS_VER).$(FS_REL).@RPM_TARGET_ARCH@.rpm
AMR_RPM = $(DEST_RPM)/sipX-freeswitch-codec-passthru-amr-$(FS_VER).$(FS_REL).@RPM_TARGET_ARCH@.rpm
G723_RPM = $(DEST_RPM)/sipX-freeswitch-codec-passthru-g723_1-$(FS_VER).$(FS_REL).@RPM_TARGET_ARCH@.rpm
G729_RPM = $(DEST_RPM)/sipX-freeswitch-codec-passthru-g729-$(FS_VER).$(FS_REL).@RPM_TARGET_ARCH@.rpm


RPMS = $(RPM) $(DEVEL_RPM) $(AMR_RPM) $(G723_RPM) $(G729_RPM)

all : $(RPMS)

$(PATCHED_SRC) :
	$(DOWNLOAD_FILE) $(SRC) $(FS_URL)/`basename $(SRC)`
	pushd $(LIBSRC) && tar -xzf $(SRC) && popd
	cp $(srcdir)/patches/bootstrap.sh $(LIBSRC)/freeswitch-1.0.rc3
	mv $(LIBSRC)/freeswitch-1.0.rc3 $(LIBSRC)/sipX-freeswitch-1.0.rc3
	pushd $(LIBSRC) && tar -czf $(PATCHED_SRC) sipX-freeswitch-1.0.rc3 && popd
	rm -rf $(LIBSRC)/sipX-freeswitch-1.0.rc3

.PHONY: build-rpms
build-rpms: $(PATCHED_SRC) freeswitch.spec 
	cp $(PATCHED_SRC) @RPMBUILD_TOPDIR@/SOURCES
	QA_RPATHS=0x0003 rpmbuild -ba --target @RPM_TARGET_ARCH@-none-linux freeswitch.spec

$(SRPM): build-rpms
	mv @RPMBUILD_TOPDIR@/SRPMS/`basename $@` $@

$(RPMS): build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/@RPM_TARGET_ARCH@/`basename $@` $@
