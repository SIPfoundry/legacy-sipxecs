# Newer versions seem to require in graphviz and graphviz
# pulls in a whole bunch of other stuff
VER = 1.10.2
REL = 1

RPMS = $(DEST_RPM)/cppunit-$(VER)-$(REL).@RPM_TARGET_ARCH@.rpm
SRPM = @RPMBUILD_TOPDIR@/SRPMS/cppunit-$(VER)-$(REL).src.rpm
SRC = $(LIBSRC)/cppunit/cppunit-$(VER).tar.gz

all: $(RPMS)

$(SRC) :
	$(DOWNLOAD_FILE) $@ $(CPPUNIT_URL)/`basename $@`

# cppunit has a rpmbuild error that some files aren't part of
# spec but are part of 'make install'. Fix that here.
.PHONY: build-rpms
build-rpms : $(SRC)
	rpmbuild -ts $(SRC)
	rpm -i $(SRPM)
	patch -p0 @RPMBUILD_TOPDIR@/SPECS/cppunit.spec < $(srcdir)/cppunit-sipx.patch
	rpmbuild -ba @RPMBUILD_TOPDIR@/SPECS/cppunit.spec

$(RPMS) : build-rpms
	cp @RPMBUILD_TOPDIR@/RPMS/@RPM_TARGET_ARCH@/`basename $@` $@
