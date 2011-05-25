include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

noinst_DATA = javac-test 

EXTRA_DIST = \
	$(test_SRC) \
	$(test_RESOURCES)

test_SRC = $(shell cd $(srcdir); find . -name '*.java')

test_DEPS = \
	$(JAVAROOT) \
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons, junit sipxcommons) \
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
