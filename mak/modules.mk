# Initial Version Copyright (C) 2010 eZuce, Inc., All Rights Reserved.
# Licensed to the User under the LGPL license.

# sipXecs projects that are essential for a running communication system.
# Order is important, projects that are dependant on other project
# should be listed after it's dependencies.  No circular dependecies
# allowed.
sipx_core = \
  sipXportLib \
  sipXtackLib \
  sipXresiprocate \
  sipXmediaLib \
  sipXmediaAdapterLib \
  sipXcallLib \
  sipXeslLib \
  sipXsupervisor \
  sipXcommserverLib \
  sipXsqa \
  sipXcommons \
  sipXrelay \
  sipXbridge \
  sipXcdr \
  sipXconfig \
  sipXopenfire \
  sipXcounterpath \
  sipXprompts \
  sipXivr \
  sipXproxy \
  sipXpublisher \
  sipXregistry \
  sipXpage \
  sipXpolycom \
  sipXrls \
  sipXsaa \
  sipXyard \
  sipXrelease \
  sipXecs

#additional configure options for sipXresiprocate package
sipXresiprocate_OPTIONS = --with-c-ares --with-ssl --with-repro --enable-ipv6 --with-tfm

# sipxecs projects that are NOT essential for a running communication system
sipx_extra = \
  sipXacccode \
  sipXcustomCallerId \
  sipXviewer \
  sipXimbot \
  sipXexample \
  sipXrest \
  sipXcallController \
  sipXcdrLog \
  sipXrecording \
  sipXhomer \
  sipXcallQueue \
  sipXtools

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
  sipXaastra \
  sipXyealink

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
  $(sipx_config) \
  $(sipx_lang)

# re: ruby-postgres, there's a new one we should be using ruby-pgsql i
# think it's called as ruby-postgres is obsoleted.
lib_all = \
  epel \
  rubygem-file-tail \
  erlang \
  freeswitch \
  hiredis \
  net-snmp \
  homer \
  openfire \
  ruby-dbi \
  cfengine \
  rubygem-net-ssh \
  rubygem-net-sftp \
  ruby-postgres \
  jasperserver \
  libjsonrpccpp

lib_exclude_fedora_16 = \
  epel \
  erlang \
  rrdtool \
  rubygem-net-ssh \
  rubygem-net-sftp

lib_exclude_fedora_17 = \
  $(lib_exclude_fedora_16) \
  ruby-postgres

lib_exclude_fedora_18 = $(lib_exclude_fedora_17)
lib_exclude_fedora_19 = $(lib_exclude_fedora_18)
lib_exclude_fedora_20 = $(lib_exclude_fedora_19)
lib_exclude_fedora_21 = $(lib_exclude_fedora_20)
lib_exclude_fedora_22 = $(lib_exclude_fedora_21)
lib_exclude_fedora_23 = $(lib_exclude_fedora_22)

lib = $(filter-out $(lib_exclude_$(DISTRO_OS)_$(DISTRO_VER)),$(lib_all))

# Project compile-time dependencies. Only list project that if
# it's dependecies were recompiled then you'd want to recompile.
freeswitch_DEPS = erlang
sipXtackLib_DEPS = sipXportLib
sipXmediaLib_DEPS = sipXtackLib
sipXmediaAdapterLib_DEPS = sipXmediaLib
sipXcallLib_DEPS = sipXmediaAdapterLib
sipXcustomCallerId_DEPS = sipXconfig
sipXcommserverLib_DEPS = sipXtackLib
sipXsqa_DEPS = sipXcommserverLib
sipXrelay_DEPS = sipXcommons
sipXbridge_DEPS = sipXcommons
sipXcdr_DEPS = sipXcommons
sipXconfig_DEPS = sipXcommons sipXcdr
sipXopenfire_DEPS = sipXconfig sipXsqa
sipXcounterpath_DEPS = sipXconfig
sipXaudiocodes_DEPS = sipXconfig
sipXivr_DEPS = sipXconfig
sipXproxy_DEPS = sipXcommserverLib
sipXpublisher_DEPS = sipXcommserverLib
sipXregistry_DEPS = sipXcommserverLib
sipXpage_DEPS = sipXcommons
sipXpolycom_DEPS = sipXconfig
sipXrls_DEPS = sipXsqa sipXcallLib sipXcommserverLib
sipXsaa_DEPS = sipXsqa sipXcallLib sipXcommserverLib
sipXhomer_DEPS = sipXsqa sipXresiprocate
sipXsbc_DEPS = sipXconfig sipXsqa sipXregistry
sipXcallQueue_DEPS = sipXconfig
sipXexample_DEPS = sipXcommserverLib sipXconfig
sipXsss_DEPS = sipXsqa sipXcommserverLib sipXresiprocate
sipXyard = sipXcommserverLib
sipXtools_DEPS = sipXtackLib sipXcommserverLib

all = \
  $(lib) \
  $(sipx)
