oacd_class_1 = \
  openacd \
  sipxopenacd

# Class 2 have 'erlang-' in tarball/src rpm name for no particular reason other 
# than they started that way.  Hopefully will change to be consistent someday.
oacd_class_2 = \
 oacd_freeswitch \
 oacd_dialplan \
 oacd_web

oacd = \
  $(oacd_class_1) \
  $(oacd_class_2)

lib += $(oacd)

openacd_VER = 2.0.0
sipxopenacd_VER = $(PACKAGE_VERSION)
$(foreach P,$(oacd_class_1), \
  $(eval $(P)_PACKAGE_REVISION = $(shell cd $(SRC)/$(P); ../config/revision-gen $($(P)_VER) 2>/dev/null || echo 'missing')) \
  $(eval $(P)_SRPM_DEFS = --define "buildno $($(P)_PACKAGE_REVISION)") \
  $(eval $(P)_RPM_DEFS = --define="buildno $($(P)_PACKAGE_REVISION)") \
  $(eval $(P)_SRPM = $(P)-$($(P)_VER)-$($(P)_PACKAGE_REVISION).src.rpm) \
  $(eval $(P)_TAR = $(SRC)/$(P)/$(P)-$($(P)_VER).tar.gz) \
  $(eval $(P)_SOURCES = $($(P)_TAR)) \
)

$(foreach P,$(oacd_class_2), \
  $(eval $(P)_VER = 2.0.0) \
  $(eval $(P)_PACKAGE_REVISION = $(shell cd $(SRC)/$(P); ../config/revision-gen $($(P)_VER) 2>/dev/null || echo 'missing')) \
  $(eval $(P)_SRPM_DEFS = --define "buildno $($(P)_PACKAGE_REVISION)") \
  $(eval $(P)_RPM_DEFS = --define="buildno $($(P)_PACKAGE_REVISION)") \
  $(eval $(P)_SRPM = erlang-$(P)-$($(P)_VER)-$($(P)_PACKAGE_REVISION).src.rpm) \
  $(eval $(P)_TAR = $(SRC)/$(P)/erlang-$(P)-$($(P)_VER).tar.gz) \
  $(eval $(P)_SOURCES = $($(P)_TAR)) \
)

openacd_DEPS = $(call deps,erlang-ejrpc2 erlang-gen_server_mock erlmongo)
# sipxopenacd is treated like a lib because does not confrom yet to sipx build policies
sipxopenacd_DEPS = $(call deps,erlmongo openacd sipXconfig)
oacd_web_DEPS = $(call deps,openacd erlang-cowboy erlang-mimetypes)
oacd_dialplan_DEPS = $(call deps,openacd)
oacd_freeswitch_DEPS = $(call deps,openacd)

openacd.dist: $(openacd_GIT_SUBMODULE)
	cd $(SRC)/$(PROJ); \
	  autoreconf -if; \
	  ./configure --disable-dep-check; \
	  make dist

.SECONDEXPANSION:
sipxopenacd.dist $(oacd_class_2:=.dist): %.dist : $$($$*_GIT_SUBMODULE)
	cd $(SRC)/$(PROJ); \
	  autoreconf -if; \
	  ./configure; \
	  make dist
