include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

jardir = @SIPX_JAVADIR@/sipXconfig
dist_jar_DATA = \
	.java-dep \
	hivemind-1.1.1.jar \
	hivemind-lib-1.1.1.jar \
	tapestry-annotations-4.1.6.jar \
	tapestry-contrib-4.1.6.jar \
	tapestry-framework-4.1.6.jar

# only used at compile time, so do not install at this time
EXTRA_DIST = \
	smartgwt-2.5.jar \
	smartgwt-skins-2.5.jar \
	sipxconfig.dart

JAR_SRCS = $(call JavaDep,$(srcdir),\
	tapestry-framework)

$(JAR_LIBSRC)/tapestry-project-%-full.zip :
	wget http://mirrors.ibiblio.org/apache//tapestry/tapestry-project-$*-full.zip

$(JAR_LIBSRC)/tapestry-framework-%.jar.src : $(JAR_LIBSRC)/tapestry-project-%-full.zip
	unzip -u $< 'tapestry-project-$*/src/tapestry-*/**'
	mv tapestry-project-$*/src $@
