freeswitch_edge_VER = 1.5.11b
freeswitch_edge_TAG = 1.5.11
freeswitch_edge_PACKAGE_REVISION = $(shell cd $(SRC)/$(PROJ); ../config/revision-gen $(freeswitch_edge_TAG))
freeswitch_edge_SRPM = freeswitch_edge-$(freeswitch_edge_VER)-$(freeswitch_edge_PACKAGE_REVISION).src.rpm
freeswitch_edge_SPEC = $(SRC)/$(PROJ)/freeswitch_edge.spec
freeswitch_edge_TARBALL = $(BUILDDIR)/$(PROJ)/freeswitch_edge-$(freeswitch_edge_VER).tar.bz2
freeswitch_edge_SOURCES = $(freeswitch_edge_TARBALL)
freeswitch_edge_INSTALL_PREFIX = /opt/sipxecs/edge/freeswitch

freeswitch_edge_SRPM_DEFS = \
  --define "build_no $(freeswitch_edge_PACKAGE_REVISION)" \
  --define "version_no $(freeswitch_edge_VER)" \
  --define "install_prefix $(freeswitch_edge_INSTALL_PREFIX)"
  
freeswitch_edge_RPM_DEFS = \
  --define "build_no $(freeswitch_edge_PACKAGE_REVISION)" \
  --define "version_no $(freeswitch_edge_VER)" \
  --define "install_prefix $(freeswitch_edge_INSTALL_PREFIX)"

freeswitch_edge.dist : $(SRC)/freeswitch_edge/.git
	test -d $(dir $(freeswitch_edge_TARBALL)) || mkdir -p $(dir $(freeswitch_edge_TARBALL))
	cd $(SRC)/$(PROJ); \
	  git archive --format tar --prefix freeswitch_edge-$(freeswitch_edge_VER)/ HEAD | bzip2 > $(freeswitch_edge_TARBALL)
