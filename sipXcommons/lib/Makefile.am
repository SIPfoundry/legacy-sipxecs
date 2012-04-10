include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

jardir = @SIPX_JAVADIR@/sipXcommons
dist_jar_DATA = \
	.java-dep \
	$(wildcard $(srcdir)/*.jar)

# Experimental makefile to download source for jars.
SPRING = \
	aop \
	asm \
	beans \
	context \
	context.support \
	expression \
	jdbc \
	orm \
	transaction \
	web \
	web.servlet

JAR_SRCS = $(call JavaDep,$(srcdir),\
	org.restlet \
	junit \
	spring-mock \
	$(foreach M,$(SPRING),org.springframework.$(M)))

$(JAR_LIBSRC)/junit-%.jar.src :
	wget https://github.com/downloads/KentBeck/junit/junit-$*-src.jar
	mv junit-$*-src.jar $@

$(JAR_LIBSRC)/org.restlet-%.jar.src : $(JAR_LIBSRC)/restlet-%.zip
	unzip -u restlet-$*.zip 'restlet-$*/src/**'
	mv restlet-$*/src $@

$(JAR_LIBSRC)/restlet-%.zip :
	wget http://www.restlet.org/downloads/1.1/restlet-$*.zip

# i'm not sure where the jar came from, but some version of the source came from the springsource src archive
$(JAR_LIBSRC)/spring-mock-2.0.4.jar.src : $(JAR_LIBSRC)/spring-3.0.5.jar.src;

# all source from the spring modules come from a single zip
$(foreach M,$(SPRING),$(JAR_LIBSRC)/org.springframework.$(M)-%.RELEASE.jar.src) : $(JAR_LIBSRC)/spring-%.jar.src;

$(JAR_LIBSRC)/spring-framework-%.RELEASE.zip :
	wget -O $@ http://s3.amazonaws.com/dist.springframework.org/release/SPR/spring-framework-$*.RELEASE.zip

$(JAR_LIBSRC)/spring-%.jar.src : $(JAR_LIBSRC)/spring-framework-%.RELEASE.zip
	unzip -u $< 'spring-framework-$*.RELEASE/projects/*/src/main/java/**'
	for d in spring-framework-$*.RELEASE/projects/*/src/main/java/org ; do \
	  rsync -rav $$d $@/; \
	done
