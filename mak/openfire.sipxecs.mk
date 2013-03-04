openfire_VER = 3.7.1
openfire_PACKAGE_REVISION = 1$(shell cd $(SRC)/$(PROJ); ../config/revision-gen $(openfire_VER))
openfire_SRPM = openfire-$(openfire_VER)-$(openfire_PACKAGE_REVISION).src.rpm
openfire_SPEC = $(SRC)/$(PROJ)/openfire.spec
openfire_TARBALL = $(BUILDDIR)/$(PROJ)/openfire-$(openfire_VER).tar.bz2
openfire_SOURCES = $(openfire_TARBALL)
openfire_DEFS = --define="OPENFIRE_VERSION $(openfire_VER)" --define="OPENFIRE_BUILDDATE $(shell date '+%a %b %e %Y')" --define="OPENFIRE_SOURCE openfire-$(openfire_VER).tar.bz2" --define="buildno $(openfire_PACKAGE_REVISION)"
openfire_SRPM_DEFS = $(openfire_DEFS)
openfire_RPM_DEFS = $(openfire_DEFS)

openfire.dist : $(openfire_GIT_SUBMODULE)
	test -d $(dir $(openfire_TARBALL)) || mkdir -p $(dir $(openfire_TARBALL))
	cd $(SRC)/$(PROJ); \
	  git archive --format tar --prefix openfire-$(openfire_VER)/ HEAD | bzip2 > $(openfire_TARBALL)
