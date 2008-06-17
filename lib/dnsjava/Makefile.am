DNSJAVA_VER = 2.0.6
DNSJAVA_REL = 1


TARBALL=dnsjava-$(DNSJAVA_VER).tar.gz

SRC = $(LIBSRC)/dnsjava/$(TARBALL)
SRPM = $(DEST_SRPM)/dnsjava-$(DNSJAVA_VER)-$(DNSJAVA_REL).src.rpm
RPMS = \
	$(DEST_RPM)/dnsjava-$(DNSJAVA_VER)-$(DNSJAVA_REL).noarch.rpm \
	$(DEST_RPM)/dnsjava-javadoc-$(DNSJAVA_VER)-$(DNSJAVA_REL).noarch.rpm
RPM_SOURCES_DIR=@RPMBUILD_TOPDIR@/SOURCES

all : $(SRPM) $(RPMS)

$(SRC) :
	$(DOWNLOAD_FILE) $@ $(DNSJAVA_URL)/`basename $@`

$(RPM_SOURCES_DIR)/$(TARBALL) : $(SRC)
	cp $< $@

$(RPM_SOURCES_DIR)/% : %
	cp $< $@

.PHONY: build-rpms
build-rpms: $(SRC) $(srcdir)/dnsjava.spec $(RPM_SOURCES_DIR)/$(TARBALL)
	rpmbuild -ba $(srcdir)/dnsjava.spec

$(SRPM): build-rpms
	mv @RPMBUILD_TOPDIR@/SRPMS/`basename $@` $@

$(RPMS): build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/noarch/`basename $@` $@

