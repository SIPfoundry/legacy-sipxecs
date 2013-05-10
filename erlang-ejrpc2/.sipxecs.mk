erlang-ejrpc2_VER = 0.0.1
erlang-ejrpc2_GITREF = da5e71c
erlang-ejrpc2_SRPM_DEFS = --define "gitref $(erlang-ejrpc2_GITREF)"
erlang-ejrpc2_RPM_DEFS = $(erlang-ejrpc2_SRPM_DEFS)
erlang-ejrpc2_REV = 2
erlang-ejrpc2_SPEC = $(SRC)/$(PROJ)/erlang-ejrpc2.spec
erlang-ejrpc2_TARBALL = jvliwanag-ejrpc2-$(erlang-ejrpc2_VER)-g$(erlang-ejrpc2_GITREF).tar.gz
erlang-ejrpc2_SOURCES = $(erlang-ejrpc2_TARBALL)
erlang-ejrpc2_SRPM = erlang-ejrpc2-$(erlang-ejrpc2_VER)-$(erlang-ejrpc2_REV)$(RPM_DIST).g$(erlang-ejrpc2_GITREF).src.rpm

erlang-ejrpc2.dist : $(DOWNLOAD_LIB_CACHE)/$(erlang-ejrpc2_TARBALL);

$(DOWNLOAD_LIB_CACHE)/$(erlang-ejrpc2_TARBALL) :
	wget -O $(DOWNLOAD_LIB_CACHE)/$(erlang-ejrpc2_TARBALL) --content-disposition https://github.com/jvliwanag/ejrpc2/tarball/$(erlang-ejrpc2_GITREF)
