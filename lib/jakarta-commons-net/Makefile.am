VER = 1.4.1
REL = 4jpp

PACKAGE=jakarta-commons-net

BUILD_OPTIONS=--without maven --without gcj_support

SRPM = $(LIBSRC)/$(PACKAGE)/$(PACKAGE)-$(VER)-$(REL).src.rpm
RPMS = $(DEST_RPM)/$(PACKAGE)-$(VER)-$(REL).noarch.rpm

all : $(RPMS)

$(SRPM) :
	$(DOWNLOAD_FILE) $@ $(JPKG_FREE_URL)/SRPMS/`basename $@`

.PHONY:build-rpms
build-rpms : $(SRPM)
	rpm -Uvh $(SRPM)
	sed --in-place --expression='s/^BuildRequires:[ \t]*java-javadoc[ \t]*$$//' \
		@RPMBUILD_TOPDIR@/SPECS/jakarta-commons-net.spec

$(RPMS) : build-rpms
	rpmbuild -ba $(BUILD_OPTIONS) @RPMBUILD_TOPDIR@/SPECS/jakarta-commons-net.spec
	cp @RPMBUILD_TOPDIR@/RPMS/noarch/`basename $@` $@
