JAIN_SIP_VER = 1.2
JAIN_SIP_REL = 1

SRC = $(LIBSRC)/jain-sip/jain-sip-$(JAIN_SIP_VER).tar.gz
SRPM = $(DEST_SRPM)/jain-sip-$(JAIN_SIP_VER)-$(JAIN_SIP_REL).src.rpm
RPMS = \
	$(DEST_RPM)/jain-sip-$(JAIN_SIP_VER)-$(JAIN_SIP_REL).noarch.rpm \
	$(DEST_RPM)/jain-sip-javadoc-$(JAIN_SIP_VER)-$(JAIN_SIP_REL).noarch.rpm
RPM_SOURCES_DIR=@RPMBUILD_TOPDIR@/SOURCES

all : $(SRPM) $(RPMS)

$(SRC) :
	$(DOWNLOAD_FILE) $@ $(JAIN_SIP_URL)/`basename $@`

$(RPM_SOURCES_DIR)/% : %
	cp $< $@

.PHONY: build-rpms
build-rpms: $(SRC) $(srcdir)/jain-sip.spec
	cp $(SRC) @RPMBUILD_TOPDIR@/SOURCES
	rpmbuild -ba $(srcdir)/jain-sip.spec

$(SRPM): build-rpms
	mv @RPMBUILD_TOPDIR@/SRPMS/`basename $@` $@

$(RPMS): build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/noarch/`basename $@` $@
