include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/src/common.am

EXTRA_DIST = \
	$(commons_SRC) \
	$(commons_RESOURCES)

noinst_DATA = \
	javac-commons \
	java-jibx

commons_SRC = \
	$(shell cd $(srcdir); find org -type f -name '*.java')

commons_RESOURCES = \
	$(shell cd $(srcdir); find -type f -not -name '*.java')

commons_DEPS = \
	$(JAVAROOT) \
	$(call JavaDep, $(top_srcdir)/lib, $(commons_PKGS)) \
	$(commons_runtime_DEPS)

jibx_DEPS = \
	$(JAVAROOT) \
	$(call JavaDep, $(top_srcdir)/lib, jibx-bind)

jibx_SRC = \
	org/sipfoundry/commons/dialoginfo/binding.xml \
	org/sipfoundry/commons/reginfo/binding.xml

jibx_SCHEMA = \
	org/sipfoundry/commons/dialoginfo/dialog-info.xsd \
	org/sipfoundry/commons/reginfo/reginfo.xsd

java-jibx-clean :
	! test -d $(JAVAROOT) || find $(JAVAROOT) -name 'JiBX_*' -exec rm {} \;
	! test -f java-compile-jibx || rm java-compile-jibx

# NOTE: if you don't clean before rebuild, second set of jibx files are generated.
java-jibx : javac-commons $(jibx_SRC) $(jibx_SCHEMA)
	$(MAKE) java-jibx-clean
	$(call JavaCommand,jibx) org.jibx.binding.Compile $(addprefix $(srcdir)/,$(jibx_SRC))
	echo timestamp > $@
	
clean-local : java-jibx-clean
CLEANFILES = java-compile-jibx
