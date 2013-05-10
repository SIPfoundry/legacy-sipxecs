# Initial Version Copyright (C) 2010 eZuce, Inc., All Rights Reserved.
# Licensed to the User under the LGPL license.

# sipxecs projects that essential for a running communication system
sipx_core = \
  sipXportLib \
  sipXtackLib \
  sipXmediaLib \
  sipXmediaAdapterLib \
  sipXcallLib \
  sipXsupervisor \
  sipXmongo \
  sipXcommserverLib \
  sipXsqa \
  sipXsnmp \
  sipXpostgres \
  sipXtunnel \
  sipXdns \
  sipXhttpd \
  sipXcommons \
  sipXrelay \
  sipXbridge \
  sipXfreeSwitch \
  sipXcdr \
  sipXacdStatistics \
  sipXconfig \
  sipXopenfire \
  sipXcounterpath \
  sipXprompts \
  sipXivr \
  sipXproxy \
  sipXpublisher \
  sipXregistry \
  sipXpark \
  sipXpage \
  sipXpolycom \
  sipXrls \
  sipXsaa \
  sipXrelease

# sipxecs projects that are NOT essential for a running communication system
sipx_extra = \
  sipXacccode \
  sipXviewer \
  sipXimbot \
  sipXexample \
  sipXrest \
  sipXcallController \
  sipXcdrLog \
  sipXevent \
  sipXrecording \
  sipXsbc \
  sipXhomer

# sipxecs projects that are NOT essential for a running communication system
# and are related to configuration system. Many are phone plugins
sipx_config = \
  sipXaudiocodes \
  sipXprovision \
  sipXcisco \
  sipXclearone \
  sipXgtek \
  sipXhitachi \
  sipXipdialog \
  sipXisphone \
  sipXnortel \
  sipXlg-nortel \
  sipXmitel \
  sipXsnom \
  sipXunidata \
  sipXgrandstream \
  sipXaastra

# Language packs not required for a function communications system
sipx_lang = \
  sipXlang-abitibi-fr_CA \
  sipXlang-ch \
  sipXlang-cs \
  sipXlang-de \
  sipXlang-en_GB \
  sipXlang-es \
  sipXlang-fr_CA \
  sipXlang-fr \
  sipXlang-it \
  sipXlang-ja \
  sipXlang-es_MX \
  sipXlang-nl \
  sipXlang-pl \
  sipXlang-pt_BR \
  sipXlang-zh

sipx_all = \
  $(sipx_core) \
  $(sipx_extra) \
  $(sipx_lang) \
  $(sipx_config)

# re: ruby-postgres, there's a new one we should be using ruby-pgsql i 
# think it's called as ruby-postgres is obsoleted.
lib_all = \
  epel-release \
  resiprocate \
  rubygem-file-tail \
  freeswitch \
  hiredis \
  net-snmp \
  homer \
  openfire \
  erlmongo \
  erlang-ejrpc2 \
  erlang-ej \
  erlang-cowboy \
  erlang-gen_server_mock \
  erlang-mimetypes \
  ruby-dbi \
  cfengine \
  oss_core \
  rrdtool \
  nsis \
  nsis-data \
  rubygem-net-ssh \
  rubygem-net-sftp \
  ruby-postgres \
  sec \
  js \
  v8 \
  mongodb \
  erlang

lib_exclude_centos_6 = \
  mongodb \
  js

lib_exclude_fedora_16 = \
  epel-release \
  erlang \
  rrdtool \
  nsis \
  nsis-data \
  rubygem-net-ssh \
  rubygem-net-sftp

lib_exclude_fedora_17 = \
  $(lib_exclude_fedora_16) \
  ruby-postgres \
  sec \
  js \
  v8 \
  mongodb

lib_exclude_fedora_18 = $(lib_exclude_fedora_17)
lib_exclude_fedora_19 = $(lib_exclude_fedora_18)
lib_exclude_fedora_20 = $(lib_exclude_fedora_19)
lib_exclude_fedora_21 = $(lib_exclude_fedora_20)
lib_exclude_fedora_22 = $(lib_exclude_fedora_21)
lib_exclude_fedora_23 = $(lib_exclude_fedora_22)

lib = $(filter-out $(lib_exclude_$(DISTRO_OS)_$(DISTRO_VER)),$(lib_all))

# sort removes dups and should speed maketime
deps = $(filter-out $(lib_exclude_$(DISTRO_OS)_$(DISTRO_VER)),$(sort $(1) $(foreach D,$(1)_DEPS,$($(D)))))

# lib deps
mongodb_DEPS = $(call deps,js)
oss_core_DEPS = $(call deps,v8)
rubygem-net-sftp_DEPS = $(call deps,rubygem-net-ssh)
gen_server_mock_DEPS = $(call deps,erlang)
erlmongo_DEPS = $(call deps,erlang)
erlang-ej_DEPS = $(call deps,erlang)
erlang-cowboy_DEPS = $(call deps,erlang)
erlang-mimetypes_DEPS = $(call deps,erlang)
erlang-ejrpc2_DEPS = $(call deps,erlang-ej)

#sipx deps
sipXportLib_DEPS = $(call deps,epel-release)
sipXtackLib_DEPS = $(call deps,sipXportLib)
sipXmediaLib_DEPS = $(call deps,sipXtackLib)
sipXmediaAdapterLib_DEPS = $(call deps,sipXmediaLib)
sipXcallLib_DEPS = $(call deps,sipXmediaAdapterLib)
sipXsupervisor_DEPS = $(call deps,sec cfengine rubygem-net-sftp)
sipXmongo_DEPS = $(call deps,mongodb sipXsupervisor)
sipXcommserverLib_DEPS = $(call deps,sipXsupervisor sipXtackLib sipXmongo sipXsnmp)
sipXsqa_DEPS = $(call deps,hiredis sipXcommserverLib)
sipXsnmp_DEPS = $(call deps,net-snmp sipXsupervisor)
sipXpostgres_DEPS = $(call deps,sipXsupervisor)
sipXtunnel_DEPS = $(call deps,sipXsupervisor)
sipXdns_DEPS = $(call deps,sipXsupervisor)
sipXhttpd_DEPS = $(call deps,sipXsupervisor)
sipXcommons_DEPS = $(call deps,sipXsupervisor)
sipXrelay_DEPS = $(call deps,sipXcommserverLib sipXcommons)
sipXbridge_DEPS = $(call deps,sipXrelay)
sipXfreeSwitch_DEPS = $(call deps,freeswitch sipXcommserverLib)
sipXcdr_DEPS = $(call deps,ruby-dbi ruby-postgres sipXcommserverLib)
sipXacdStatistics_DEPS = $(call deps,ruby-dbi ruby-postgres sipXcommons)
sipXconfig_DEPS = $(call deps,sipXcommons sipXsupervisor sipXacdStatistics sipXcdr sipXpostgres sipXcommserverLib sipXhttpd sipXmongo)
sipXopenfire_DEPS = $(call deps,openfire sipXconfig)
sipXcounterpath_DEPS = $(call deps,sipXconfig)
sipXaudiocodes_DEPS = $(call deps,sipXconfig)
sipXprompts_DEPS = $(call deps,sipXsupervisor)
sipXivr_DEPS = $(call deps,sipXconfig)
sipXproxy_DEPS = $(call deps,sipXcommserverLib)
sipXpublisher_DEPS = $(call deps,sipXcommserverLib)
sipXregistry_DEPS = $(call deps,sipXcommserverLib)
sipXpark_DEPS = $(call deps,sipXcallLib sipXcommserverLib)
sipXpage_DEPS = $(call deps,sipXcommserverLib sipXcommons)
sipXpolycom_DEPS = $(call deps,sipXconfig)
sipXrls_DEPS = $(call deps,sipXsqa sipXcallLib sipXcommserverLib)
sipXsaa_DEPS = $(call deps,sipXcallLib)
sipXhomer_DEPS = $(call deps,homer resiprocate sipXsqa)
sipXsbc_DEPS = $(call deps,oss_core sipXsupervisor sipXconfig sipXsqa sipXregistry)
sipXrelease_DEPS =
sipXviewer_DEPS = $(call deps,nsis nsis-data)

# uc
sipXtest_DEPS = $(call deps,rrdtool)

all = \
  $(lib) \
  $(sipx)
