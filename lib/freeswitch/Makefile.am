FS_VER = 1.0.0
FS_REL = 1
FS_URL=http://files.freeswitch.org

SRC = $(LIBSRC)/freeswitch-$(FS_VER).tar.gz
PATCHED_SRC = $(LIBSRC)/sipx-freeswitch-$(FS_VER).tar.gz

RPM = $(DEST_RPM)/sipx-freeswitch-$(FS_VER)-$(FS_REL).@RPM_TARGET_ARCH@.rpm
SRPM = $(DEST_RPM)/sipx-freeswitch-$(FS_VER)-$(FS_REL).src.rpm
DEVEL_RPM = $(DEST_RPM)/sipx-freeswitch-devel-$(FS_VER)-$(FS_REL).@RPM_TARGET_ARCH@.rpm
AMR_RPM = $(DEST_RPM)/sipx-freeswitch-codec-passthru-amr-$(FS_VER)-$(FS_REL).@RPM_TARGET_ARCH@.rpm
G723_RPM = $(DEST_RPM)/sipx-freeswitch-codec-passthru-g723_1-$(FS_VER)-$(FS_REL).@RPM_TARGET_ARCH@.rpm
G729_RPM = $(DEST_RPM)/sipx-freeswitch-codec-passthru-g729-$(FS_VER)-$(FS_REL).@RPM_TARGET_ARCH@.rpm


RPMS = $(RPM) $(DEVEL_RPM) $(AMR_RPM) $(G723_RPM) $(G729_RPM)

all : $(RPMS)

$(PATCHED_SRC) :
	$(DOWNLOAD_FILE) $(SRC) $(FS_URL)/`basename $(SRC)`
	pushd $(LIBSRC) && tar -xzf $(SRC) && popd
	cp $(srcdir)/patches/bootstrap.sh $(LIBSRC)/freeswitch-$(FS_VER)
	mv $(LIBSRC)/freeswitch-$(FS_VER) $(LIBSRC)/sipx-freeswitch-$(FS_VER)
	pushd $(LIBSRC) && tar -czf $(PATCHED_SRC) sipx-freeswitch-$(FS_VER) && popd
	rm -rf $(LIBSRC)/sipx-freeswitch-$(FS_VER)

.PHONY: build-rpms
build-rpms: $(PATCHED_SRC) freeswitch.spec 
	cp $(PATCHED_SRC) @RPMBUILD_TOPDIR@/SOURCES
	QA_RPATHS=0x0003 rpmbuild -ba --target @RPM_TARGET_ARCH@-none-linux freeswitch.spec

$(SRPM): build-rpms
	mv @RPMBUILD_TOPDIR@/SRPMS/`basename $@` $@

$(RPMS): build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/@RPM_TARGET_ARCH@/`basename $@` $@
