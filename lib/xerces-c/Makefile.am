XERCES_C_VER = 2.8.0
XERCES_C_REL = 1

VER_UNDERSCORE = $(subst .,_,$(XERCES_C_VER))
SRC = $(LIBSRC)/xerces-c/xerces-c-src_$(VER_UNDERSCORE).tar.gz
SRPM = $(DEST_SRPM)/xerces-c-$(XERCES_C_VER)-$(XERCES_C_REL).src.rpm
RPMS = \
	$(DEST_RPM)/xerces-c-$(XERCES_C_VER)-$(XERCES_C_REL).@RPM_TARGET_ARCH@.rpm \
	$(DEST_RPM)/xerces-c-devel-$(XERCES_C_VER)-$(XERCES_C_REL).@RPM_TARGET_ARCH@.rpm

all : $(SRPM) $(RPMS)


$(SRC) :
	$(DOWNLOAD_FILE) $@ $(XERCES_C_URL)/`basename $@`

.PHONY: build-rpms
build-rpms: $(SRC)
	cp $(SRC) @RPMBUILD_TOPDIR@/SOURCES
	tar -xzf $(SRC) -O xerces-c-src_$(VER_UNDERSCORE)/xerces-c.spec > ../xerces-c.spec
	rpmbuild -ba --target @RPM_TARGET_ARCH@-none-linux $(srcdir)/xerces-c.spec

$(SRPM): build-rpms
	mv @RPMBUILD_TOPDIR@/SRPMS/`basename $@` $@

$(RPMS): build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/@RPM_TARGET_ARCH@/`basename $@` $@
