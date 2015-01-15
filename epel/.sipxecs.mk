# Initial Version Copyright (C) 2013 eZuce, Inc., All Rights Reserved.
# Licensed to the User under the LGPL license.

# EPEL changes are more dramatic then CentOS base. Major new versions are introduced
# at any given moment and when it happens, sipXecs has to rush out new support.
#   See http://track.sipfoundry.org/browse/UC-1664
# By bundling EPEL rpms into sipXecs, we're more in control of when rpms upgrade but
# we need to make it easy to stay somewhat in sync w/EPEL.

ifeq ($(DISTRO_ARCH),x86_64)
EXCLUDE_ARCH='--exclude=*.i686.rpm'
endif

epel.dist epel.srpm:;

epel.rpm :
	rsync -av \
	  $(EXCLUDE_ARCH) \
	  $(addprefix $(CENTOS_RSYNC_URL)/epel/6/$(DISTRO_ARCH)/,$(RUNTIME_EPEL) $(BUILD_EPEL)) \
	  $(MOCK_RESULTS_DIR)/
	mock $(MOCK_OPTS) --scrub=cache
	$(MAKE) repo-dedup
	cd $(MOCK_RESULTS_DIR); createrepo $(CREATEREPO_OPTS) .
	$(SRC)/tools/dep save $(DISTRO).$(PROJ).rpm $(SRC)/$(PROJ)

epel.clean :
	-rm $(addprefix $(MOCK_RESULTS_DIR)/,$(BUILD_EPEL) $(RUNTIME_EPEL))

# Only used for building, not required for ISO
BUILD_EPEL = \
	bakefile-* \
	c-ares19-* \
	c-ares19-devel-* \
	Canna-libs-* \
	ccache-* \
	compface-1.5*\
	erlang-erlydtl-* \
	erlang-getopt-* \
	erlang-gettext-* \
	erlang-lfe-* \
	erlang-mustache-* \
	erlang-neotoma-* \
	erlang-protobuffs-* \
	erlang-meck-* \
	erlang-rebar-* \
	gperftools-devel-* \
	gtest-devel-* \
	gyp-* \
	http-parser-* \
	http-parser-devel-* \
	libdnet-* \
	libdnet-devel-* \
	libev-* \
	libmongodb-devel-2.4* \
	libuv-* \
	libuv-devel-* \
	neXtaw-* \
	node-gyp-* \
	nodejs-* \
	npm-* \
	poco-* \
	poco-debug-* \
	python-empy-* \
	rubygem-mocha-* \
	v8-* \
	v8-devel-* \
	wxGTK-devel-* \
	wxGTK-media-* \
	xemacs-* \
	leveldb-devel-*

# Technincally these could be build and runtime requirements
RUNTIME_EPEL = \
	erlang-lager-* \
	erlang-gen_leader-* \
	erlang-gproc-* \
	erlang-erlando-* \
	erlang-ibrowse-* \
	erlang-rpm-macros-* \
	fail2ban-* \
	gperftools-libs-* \
	gtest-1* \
	js-* \
	libiodbc-3* \
	libev-4.* \
	libmcrypt-* \
	libmongodb-2.4* \
	libunwind-* \
	monit-* \
	mongodb-2.4* \
	mongodb-server-2.4* \
	openpgm-5* \
	php-pecl-mongo-* \
	poco-crypto-* \
	poco-data-* \
	poco-foundation-* \
	poco-mysql-* \
	poco-net-* \
	poco-netssl-* \
	poco-odbc-* \
	poco-pagecompiler-* \
	poco-sqlite-* \
	poco-util-* \
	poco-xml-* \
	poco-zip-* \
	python-argparse-* \
	python-bson-2.5* \
	python-inotify-0* \
	python-pymongo-2.5* \
	redis-* \
	rubygem-daemons-* \
	sec-* \
	shorewall-4* \
	shorewall-core-* \
	snappy-* \
	v8-3* \
	wxBase-* \
	wxGTK-2.* \
	wxGTK-gl-* \
	zeromq-* \
	leveldb-* \
	python-pymongo-gridfs-2.5* \
	sipp-* \
	socat-1.7.*

