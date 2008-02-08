RRDTOOL_VER = 1.2.26
RRDTOOL_REL = 5

SRC = $(LIBSRC)/rrdtool/rrdtool-$(RRDTOOL_VER).tar.gz
SRPM = $(DEST_SRPM)/rrdtool-$(RRDTOOL_VER)-$(RRDTOOL_REL).src.rpm
RPMS = \
	$(DEST_RPM)/rrdtool-$(RRDTOOL_VER)-$(RRDTOOL_REL).@RPM_TARGET_ARCH@.rpm \
	$(DEST_RPM)/rrdtool-perl-$(RRDTOOL_VER)-$(RRDTOOL_REL).@RPM_TARGET_ARCH@.rpm
	$(DEST_RPM)/rrdtool-devel-$(RRDTOOL_VER)-$(RRDTOOL_REL).@RPM_TARGET_ARCH@.rpm

all : $(SRPM) $(RPMS)


$(SRC) :
	$(DOWNLOAD_FILE) $@ $(RRDTOOL_URL)/`basename $@`

.PHONY: build-rpms
build-rpms: $(SRC) $(srcdir)/rrdtool.spec
	cp $(SRC) @RPMBUILD_TOPDIR@/SOURCES
	rpmbuild -ba --target @RPM_TARGET_ARCH@-none-linux $(srcdir)/rrdtool.spec

$(SRPM): build-rpms
	mv @RPMBUILD_TOPDIR@/SRPMS/`basename $@` $@

$(RPMS): build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/@RPM_TARGET_ARCH@/`basename $@` $@
