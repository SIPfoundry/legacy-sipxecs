erlang-cowboy_VER = 0.6.1
erlang-cowboy_GITREF = af07e04
erlang-cowboy_SRPM_DEFS = --define "gitref $(erlang-cowboy_GITREF)"
erlang-cowboy_RPM_DEFS = $(erlang-cowboy_SRPM_DEFS)
erlang-cowboy_REV = 1
erlang-cowboy_SPEC = $(SRC)/$(PROJ)/erlang-cowboy.spec
erlang-cowboy_TARBALL = extend-cowboy-$(erlang-cowboy_VER)-g$(erlang-cowboy_GITREF).tar.gz
erlang-cowboy_SOURCES = $(erlang-cowboy_TARBALL)
erlang-cowboy_SRPM = erlang-cowboy-$(erlang-cowboy_VER)-$(erlang-cowboy_REV)$(RPM_DIST).g$(erlang-cowboy_GITREF).src.rpm

erlang-cowboy.dist : $(DOWNLOAD_LIB_CACHE)/$(erlang-cowboy_TARBALL);

# Helpful for package maintainer when you want to pull in new changes from erlang-cowboy project
$(DOWNLOAD_LIB_CACHE)/$(erlang-cowboy_TARBALL) :
	wget -O $(DOWNLOAD_LIB_CACHE)/$(erlang-cowboy_TARBALL) --content-disposition https://github.com/extend/cowboy/tarball/$(erlang-cowboy_GITREF)
