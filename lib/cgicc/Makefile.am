CGICC_VER = 3.2.3
CGICC_REL = 3

SRC = $(LIBSRC)/cgicc/cgicc-$(CGICC_VER).tar.gz
SRPM = $(DEST_SRPM)/cgicc-$(CGICC_VER)-$(CGICC_REL).src.rpm
RPMS = \
	$(DEST_RPM)/cgicc-$(CGICC_VER)-$(CGICC_REL).@RPM_TARGET_ARCH@.rpm \
	$(DEST_RPM)/cgicc-devel-$(CGICC_VER)-$(CGICC_REL).@RPM_TARGET_ARCH@.rpm
PATCH = cgicc-3.2.3_64bit.patch
RPM_SOURCES_DIR=@RPMBUILD_TOPDIR@/SOURCES

all : $(SRPM) $(RPMS)


$(SRC) :
	$(DOWNLOAD_FILE) $@ $(CGICC_URL)/`basename $@`

$(RPM_SOURCES_DIR)/% : %
	cp $< $@

.PHONY: build-rpms
build-rpms: $(SRC) $(srcdir)/cgicc.spec $(RPM_SOURCES_DIR)/$(PATCH)
	cp $(SRC) @RPMBUILD_TOPDIR@/SOURCES
	rpmbuild -ba --target @RPM_TARGET_ARCH@-none-linux $(srcdir)/cgicc.spec

$(SRPM): build-rpms
	mv @RPMBUILD_TOPDIR@/SRPMS/`basename $@` $@

$(RPMS): build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/@RPM_TARGET_ARCH@/`basename $@` $@
