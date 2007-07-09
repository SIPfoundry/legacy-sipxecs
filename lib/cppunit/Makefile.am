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
# unpack and patch the spec file and build from it instead of directly from the tar.gz
.PHONY: build-rpms
build-rpms : $(SRC)
	tar xfzO $(SRC) cppunit-$(VER)/cppunit.spec > $(shell rpm --eval '%{_topdir}')/SPECS/cppunit.spec
	cp $(SRC) $(shell rpm --eval '%{_topdir}')/SOURCES/
	patch -p0 $(shell rpm --eval '%{_topdir}')/SPECS/cppunit.spec < $(srcdir)/cppunit-sipx.patch
	rpmbuild -ba @RPMBUILD_TOPDIR@/SPECS/cppunit.spec

$(RPMS) : build-rpms
	cp @RPMBUILD_TOPDIR@/RPMS/@RPM_TARGET_ARCH@/`basename $@` $@
