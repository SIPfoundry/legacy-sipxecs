VER = 1.4.1
REL = 4jpp

PACKAGE=jakarta-commons-net

BUILD_OPTIONS=--without maven --without gcj_support

SRPM = $(LIBSRC)/$(PACKAGE)/$(PACKAGE)-$(VER)-$(REL).src.rpm
RPMS = \
	$(DEST_RPM)/$(PACKAGE)-$(VER)-$(REL).noarch.rpm

all : $(RPMS)

$(SRPM) :
	$(DOWNLOAD_FILE) $@ $(JPKG_FREE_URL)/SRPMS/`basename $@`

.PHONY:build-rpms
build-rpms : $(SRPM)
	rpmbuild $(BUILD_OPTIONS) --rebuild $(SRPM)

$(RPMS) : build-rpms
	cp @RPMBUILD_TOPDIR@/RPMS/noarch/`basename $@` $@

