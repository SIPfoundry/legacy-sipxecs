XERCES_C_LIBSRC=$(LIBSRC)/xerces-c

VER = 2.7.0
REL = 3

RPMS = \
	$(DEST_RPM)/xerces-c-$(VER)-$(REL).@RPM_TARGET_ARCH@.rpm \
	$(DEST_RPM)/xerces-c-devel-$(VER)-$(REL).@RPM_TARGET_ARCH@.rpm

VER_UNDERSCORE = $(subst .,_,$(VER))
SRC = $(XERCES_C_LIBSRC)/xerces-c-src_$(VER_UNDERSCORE).tar.gz

all : $(RPMS)

$(SRC) :
	$(DOWNLOAD_FILE) $@ $(XERCES_C_URL)/`basename $@`

# Patch the spec file, rpmbuild complains when building it
.PHONY: build-rpms
build-rpms : $(SRC)
	cp $(SRC) @RPMBUILD_TOPDIR@/SOURCES
	tar -xzf $(SRC) -O xerces-c-src_$(VER_UNDERSCORE)/xerces-c.spec > xerces-c.spec.orig
	patch -p0 xerces-c.spec.orig < $(srcdir)/xerces-c-sipx.patch \
	    -o @RPMBUILD_TOPDIR@/SPECS/xerces-c.spec
	rpmbuild -ba @RPMBUILD_TOPDIR@/SPECS/xerces-c.spec

$(RPMS) : build-rpms
	cp @RPMBUILD_TOPDIR@/RPMS/@RPM_TARGET_ARCH@/`basename $@` $@
