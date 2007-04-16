# There are much newer versions, however PCRE is only required for stale distros
# like RHE4 so little reason to risk upgrading IMO.  
# pcre-devel that comes w/RHE4 installs pcre into /usr/include/pcre/pcre.h and we need it
# in /usr/include/pcre.h
VER = 4.5
REL = 2

SRC = $(LIBSRC)/pcre/pcre-$(VER).tar.bz2

RPMS = \
	$(DEST_RPM)/pcre-$(VER)-$(REL).@RPM_TARGET_ARCH@.rpm \
	$(DEST_RPM)/pcre-devel-$(VER)-$(REL).@RPM_TARGET_ARCH@.rpm

all : $(SRC) $(RPMS)

$(SRC) :
	$(DOWNLOAD_FILE) $@ $(PCRE_URL)/`basename $@`

.PHONY: build-rpms
build-rpms : $(SRC) $(srcdir)/pcre-3.9-libtool.patch $(srcdir)/pcre.spec
	cp $(SRC) @RPMBUILD_TOPDIR@/SOURCES
	cp $(srcdir)/pcre-3.9-libtool.patch @RPMBUILD_TOPDIR@/SOURCES
	rpmbuild -bb --target @RPM_TARGET_ARCH@-none-linux $(srcdir)/pcre.spec

$(RPMS) : build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/@RPM_TARGET_ARCH@/`basename $@` $@
