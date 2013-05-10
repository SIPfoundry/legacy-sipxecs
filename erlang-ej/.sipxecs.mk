erlang-ej_VER = 0.0.2
erlang-ej_GITREF = c33ab74
erlang-ej_SRPM_DEFS = --define "gitref $(erlang-ej_GITREF)"
erlang-ej_RPM_DEFS = $(erlang-ej_SRPM_DEFS)
erlang-ej_REV = 2
erlang-ej_SPEC = $(SRC)/$(PROJ)/erlang-ej.spec
erlang-ej_TARBALL = seth-ej-$(erlang-ej_VER)-0-g$(erlang-ej_GITREF).tar.gz
erlang-ej_SOURCES = \
	$(erlang-ej_TARBALL) \
	$(SRC)/$(PROJ)/erlang-ej-0001-Replace-git-based-app-vsn.patch
erlang-ej_SRPM = erlang-ej-$(erlang-ej_VER)-$(erlang-ej_REV)$(RPM_DIST).g$(erlang-ej_GITREF).src.rpm

erlang-ej.dist : $(erlang-ej_TARBALL);

# Helpful for package maintainer when you want to pull in new changes from erlang-ej project
$(erlang-ej_TARBALL) :
	wget -O $(DOWNLOAD_LIB_CACHE)/$(erlang-ej_TARBALL) --content-disposition https://github.com/seth/ej/tarball/$(erlang-ej_GITREF)
