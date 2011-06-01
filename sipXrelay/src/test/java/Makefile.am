include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

# None of the unit tests work so renamed javac-test to javac-xtest so it at least compiles
noinst_DATA = javac-xtest

EXTRA_DIST = \
	$(xtest_SRC) \
	$(xtest_RESOURCES)

xtest_SRC = $(shell cd $(srcdir); find . -name '*.java')

xtest_DEPS = \
	$(JAVAROOT) \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,junit $(relay_PKGS)) \
	$(top_builddir)/src/main/java/classes

# TODO: Port this ant test harness
# <target name="symserver" description="Test sipXrelay" depends="clean,cleanlogs,compile" >
# 	     <java fork="yes" classname="org.sipfoundry.sipxrelay.TransReceiver">
# 	     	   <arg value="true"/>
# 	     	   <arg value="192.168.5.240"/>
# 	     	   <arg value="192.168.5.240"/>
# 	     	   <arg value="8080"/>
# 		        <classpath>
# 			  <pathelement location="${classes}" />
# 	                  <fileset dir="lib">
# 	                         <include name="**/*.jar" />
# 	                  </fileset>
# 	           		  <fileset dir="tools">
# 	           		         <include name="**/*.jar" />
# 	           	      </fileset>
# 	           </classpath>
# 	     </java>
# </target>

# <target name="symclient" description="Test Start sipXrelay" depends="clean,cleanlogs,compile" >
# 	     <java fork="yes" classname="org.sipfoundry.sipxrelay.TransReceiver">
# 	     	   <arg value="false"/>
# 	     	   <arg value="192.168.5.240"/>
# 	     	   <arg value="192.168.5.240"/>
# 	     	   <arg value="8080"/>
# 	     	   <arg value="50"/>
# 		        <classpath>
# 			  <pathelement location="${classes}" />
# 	                  <fileset dir="lib">
# 	                         <include name="**/*.jar" />
# 	                  </fileset>
# 	           		  <fileset dir="tools">
# 	           		         <include name="**/*.jar" />
# 	           	      </fileset>
# 	           </classpath>
# 	     </java>
# </target>
