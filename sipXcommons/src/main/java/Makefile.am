include $(top_srcdir)/config/java.am

java_modules = \
	commons \
	jibx

EXTRA_DIST = \
	$(commons_SRC) \
	$(commons_RESOURCES)

commons_SRC = \
	$(shell find @srcdir@ -type f -name '*.java')

commons_RESOURCES = \
	$(shell find $(srcdir) -type f -not -name '*.java')

commons_DEPS = \
	$(JAVAROOT) \
	$(shell find $(top_srcdir)/lib -type f -name '*.jar')

# technically only a small subset, but more is no matter
jibx_DEPS = $(commons_DEPS)

jibx_SRC = \
	org/sipfoundry/commons/dialoginfo/binding.xml \
	org/sipfoundry/commons/reginfo/binding.xml

jibx_SCHEMA = \
	org/sipfoundry/commons/dialoginfo/dialog-info.xsd \
	org/sipfoundry/commons/reginfo/reginfo.xsd

java-clean-jibx :
	find $(JAVAROOT) -name 'JiBX_*' -exec rm {} \;
	! test -f java-compile-jibx || rm java-compile-jibx

# NOTE: if you don't clean before rebuild, second set of jibx files are generated.
java-compile-jibx : java-compile-commons $(jibx_SRC) $(jibx_SCHEMA)
	$(MAKE) java-clean-jibx
	$(call ClassPathEnv,$(jibx_DEPS)) $(JAVA) org.jibx.binding.Compile $(foreach F,$(jibx_SRC),$(srcdir)/$(F))
	echo timestamp > $@
