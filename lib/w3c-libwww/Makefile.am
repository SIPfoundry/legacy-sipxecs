W3C_LIBSRC = $(LIBSRC)/w3c-libwww

# NOTE: these values are duplicated in sipx-w3c-libwww.spec
VER = 5.4.0
REL = 3

SRC = $(W3C_LIBSRC)/w3c-libwww-$(VER).tgz

SIPX_SRC = @RPMBUILD_TOPDIR@/SOURCES/sipx-w3c-libwww-$(VER).tgz

RPMS = \
	$(DEST_RPM)/sipx-w3c-libwww-$(VER)-$(REL).@RPM_TARGET_ARCH@.rpm \
	$(DEST_RPM)/sipx-w3c-libwww-devel-$(VER)-$(REL).@RPM_TARGET_ARCH@.rpm

ORIG_RPM = $(subst sipx-w3c,w3c,`basename $@`)

all : $(RPMS)

$(SRC) :
	$(DOWNLOAD_FILE) $@ $(W3C_SRC_URL)/`basename $@`

# untar and tar to change the root path
$(SIPX_SRC) : $(SRC)
	rm -rf /tmp/w3c-libwww-$(VER) 2>/dev/null
	tar -C /tmp -xzf $(SRC)
	rm -rf /tmp/sipx-w3c-libwww-$(VER) 2>/dev/null
	mv -f /tmp/w3c-libwww-$(VER) /tmp/sipx-w3c-libwww-$(VER)
	tar -C /tmp -czf $@ sipx-w3c-libwww-$(VER)
	rm -rf /tmp/sipx-w3c-libwww-$(VER)

.PHONY: build-rpms
build-rpms : $(SIPX_SRC) $(srcdir)/sipx-w3c-libwww.spec
	rpmbuild -ba $(srcdir)/sipx-w3c-libwww.spec

$(RPMS) : build-rpms
	cp @RPMBUILD_TOPDIR@/RPMS/@RPM_TARGET_ARCH@/`basename $@` $@
