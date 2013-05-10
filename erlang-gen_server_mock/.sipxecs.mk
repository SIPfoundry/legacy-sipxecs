erlang-gen_server_mock_VER = 0.0.5
erlang-gen_server_mock_GITREF = 37cb2b3
erlang-gen_server_mock_SRPM_DEFS = --define "gitref $(erlang-gen_server_mock_GITREF)"
erlang-gen_server_mock_RPM_DEFS = $(erlang-gen_server_mock_SRPM_DEFS)
erlang-gen_server_mock_REV = 1
erlang-gen_server_mock_SPEC = $(SRC)/$(PROJ)/erlang-gen_server_mock.spec
erlang-gen_server_mock_TARBALL = sipxopenacd-gen_server_mock-$(erlang-gen_server_mock_VER)-g$(erlang-gen_server_mock_GITREF).tar.gz
erlang-gen_server_mock_SOURCES = $(erlang-gen_server_mock_TARBALL)
erlang-gen_server_mock_SRPM = erlang-gen_server_mock-$(erlang-gen_server_mock_VER)-$(erlang-gen_server_mock_REV)$(RPM_DIST).g$(erlang-gen_server_mock_GITREF).src.rpm

erlang-gen_server_mock.dist : $(DOWNLOAD_LIB_CACHE)/$(erlang-gen_server_mock_TARBALL);

$(DOWNLOAD_LIB_CACHE)/$(erlang-gen_server_mock_TARBALL) :
	wget -O $(DOWNLOAD_LIB_CACHE)/$(erlang-gen_server_mock_TARBALL) --content-disposition https://github.com/sipxopenacd/gen_server_mock/tarball/$(erlang-gen_server_mock_GITREF)
