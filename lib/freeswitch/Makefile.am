FS_VER = 1.0
FS_REL = rc3-1
FS_URL=http://files.freeswitch.org

SRC = $(LIBSRC)/freeswitch-1.0.rc3.tar.gz

RPM = $(DEST_RPM)/freeswitch-$(FS_VER).$(FS_REL).@RPM_TARGET_ARCH@.rpm
SRPM = $(DEST_RPM)/freeswitch-$(FS_VER).$(FS_REL).src.rpm
DEVEL_RPM = $(DEST_RPM)/freeswitch-devel-$(FS_VER).$(FS_REL).@RPM_TARGET_ARCH@.rpm
AMR_RPM = $(DEST_RPM)/freeswitch-codec-passthru-amr-$(FS_VER).$(FS_REL).@RPM_TARGET_ARCH@.rpm
G723_RPM = $(DEST_RPM)/freeswitch-codec-passthru-g723_1-$(FS_VER).$(FS_REL).@RPM_TARGET_ARCH@.rpm
G729_RPM = $(DEST_RPM)/freeswitch-codec-passthru-g729-$(FS_VER).$(FS_REL).@RPM_TARGET_ARCH@.rpm


RPMS = $(RPM) $(DEVEL_RPM) $(AMR_RPM) $(G723_RPM) $(G729_RPM)

all : $(RPMS)

$(SRC) :
	$(DOWNLOAD_FILE) $@ $(FS_URL)/`basename $@`
	pushd $(LIBSRC) && tar -xzf $(SRC) && popd
	cp $(srcdir)/patches/bootstrap.sh $(LIBSRC)/freeswitch-1.0.rc3
	rm $(SRC)
	pushd $(LIBSRC) && tar -czf $(SRC) freeswitch-1.0.rc3 && popd
	rm -rf $(LIBSRC)/freeswitch-1.0.rc3


.PHONY: build-rpms
build-rpms: $(SRC) $(srcdir)/freeswitch.spec 
	cp $(SRC) @RPMBUILD_TOPDIR@/SOURCES
	QA_RPATHS=0x0003 rpmbuild -ba --target @RPM_TARGET_ARCH@-none-linux $(srcdir)/freeswitch.spec

$(SRPM): build-rpms
	mv @RPMBUILD_TOPDIR@/SRPMS/`basename $@` $@

$(RPMS): build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/@RPM_TARGET_ARCH@/`basename $@` $@
