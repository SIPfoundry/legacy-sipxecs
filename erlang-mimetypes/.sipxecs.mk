erlang-mimetypes_VER = 0.9
erlang-mimetypes_GITREF = d42a72a
erlang-mimetypes_SRPM_DEFS = --define "gitref $(erlang-mimetypes_GITREF)"
erlang-mimetypes_RPM_DEFS = $(erlang-mimetypes_SRPM_DEFS)
erlang-mimetypes_REV = 1
erlang-mimetypes_SPEC = $(SRC)/$(PROJ)/erlang-mimetypes.spec
erlang-mimetypes_TARBALL = spawngrid-mimetypes-$(erlang-mimetypes_VER)-g$(erlang-mimetypes_GITREF).tar.gz
erlang-mimetypes_SOURCES = \
	$(erlang-mimetypes_TARBALL) \
	$(SRC)/$(PROJ)/erlang-mimetypes-0001-Replace-git-vsn-with-fixed-value.patch
erlang-mimetypes_SRPM = erlang-mimetypes-$(erlang-mimetypes_VER)-$(erlang-mimetypes_REV)$(RPM_DIST).g$(erlang-mimetypes_GITREF).src.rpm

erlang-mimetypes.dist : $(DOWNLOAD_LIB_CACHE)/$(erlang-mimetypes_TARBALL);
$(DOWNLOAD_LIB_CACHE)/$(erlang-mimetypes_TARBALL) :
	wget -O $@ --content-disposition https://github.com/spawngrid/mimetypes/tarball/$(erlang-mimetypes_GITREF)
