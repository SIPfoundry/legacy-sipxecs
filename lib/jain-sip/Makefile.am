JAIN_SIP_VER = 1.2.80
JAIN_SIP_REL = 1


TARBALL=jain-sip-src-$(JAIN_SIP_VER).tar.gz

SRC = $(LIBSRC)/jain-sip/$(TARBALL)
SRPM = $(DEST_SRPM)/jain-sip-$(JAIN_SIP_VER)-$(JAIN_SIP_REL).src.rpm
RPMS = \
	$(DEST_RPM)/jain-sip-$(JAIN_SIP_VER)-$(JAIN_SIP_REL).noarch.rpm \
	$(DEST_RPM)/jain-sip-javadoc-$(JAIN_SIP_VER)-$(JAIN_SIP_REL).noarch.rpm
RPM_SOURCES_DIR=@RPMBUILD_TOPDIR@/SOURCES

all : $(SRPM) $(RPMS)

$(SRC) :
	$(DOWNLOAD_FILE) $@ $(JAIN_SIP_URL)/`basename $@`

$(RPM_SOURCES_DIR)/$(TARBALL) : $(SRC)
	cp $< $@

$(RPM_SOURCES_DIR)/% : %
	cp $< $@

.PHONY: build-rpms
build-rpms: $(SRC) $(srcdir)/jain-sip.spec $(RPM_SOURCES_DIR)/$(TARBALL)
	rpmbuild -ba $(srcdir)/jain-sip.spec

$(SRPM): build-rpms
	mv @RPMBUILD_TOPDIR@/SRPMS/`basename $@` $@

$(RPMS): build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/noarch/`basename $@` $@
