CGICC_VER = 3.2.7
CGICC_REL = 2

SRC = $(srcdir)/src/cgicc-$(CGICC_VER).tar.gz
SRPM = $(DEST_SRPM)/cgicc-$(CGICC_VER)-$(CGICC_REL).src.rpm
RPMS = \
	$(DEST_RPM)/cgicc-$(CGICC_VER)-$(CGICC_REL).@RPM_TARGET_ARCH@.rpm \
	$(DEST_RPM)/cgicc-devel-$(CGICC_VER)-$(CGICC_REL).@RPM_TARGET_ARCH@.rpm
RPM_SOURCES_DIR=@RPMBUILD_TOPDIR@/SOURCES

SPEC_FILE = $(srcdir)/src/cgicc.spec

all : $(SRPM) $(RPMS)


$(RPM_SOURCES_DIR)/% : %
	cp $< $@

.PHONY: build-rpms
build-rpms: 
	cp $(SRC) @RPMBUILD_TOPDIR@/SOURCES
	cp $(srcdir)/src/*patch @RPMBUILD_TOPDIR@/SOURCES
	rpmbuild -ba --target @RPM_TARGET_ARCH@-none-linux $(SPEC_FILE)

$(SRPM): build-rpms
	mv @RPMBUILD_TOPDIR@/SRPMS/`basename $@` $@

$(RPMS): build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/@RPM_TARGET_ARCH@/`basename $@` $@
