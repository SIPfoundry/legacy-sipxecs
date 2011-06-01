include config/utility.am
include config/project.am

@PACKAGE@_EXTRA_DIST = \
	@PACKAGE@-rpmlintrc \
	LICENSE \
	NOTICE \
	meta

SUBDIRS = \
	bin \
	etc \
	lib \
	neoconf \
	report \
	web \
	.

install-data-hook :
	$(mkdir_p) $(DESTDIR)@SIPX_VARDIR@/upload
	$(mkdir_p) $(DESTDIR)@SIPX_VARDIR@/backup

precommit :
	$(error To be implemented)

reset-db reset-test-db :
	$(error To be implemented)


precommit check-integration check-performance check-db :
	$(error To be implemented)

# <target name="reset-acd-history-db">
#   <property name="acd.history.test.db.url" value="jdbc:postgresql://localhost/${acd.history.test.db}" />
#   <exec executable="dropdb" outputproperty="eatme">
#     <arg line="-U ${sipxconfig.db.user} ${acd.history.test.db}" />
#   </exec>
#   <exec executable="createdb" outputproperty="eatme">
#     <arg line="-U ${sipxconfig.db.user} ${acd.history.test.db}" />
#   </exec>
#   <pgsql url="${acd.history.test.db.url}" src="${acd.history.db.schema}" />
# 	<pgsql url="${acd.history.test.db.url}" src="${acd.history.db.enableUpgradesPatch}" />
# 	<pgsql url="${acd.history.test.db.url}" src="${acd.history.db.locationFqdnPatch}" />
#   <pgsql url="${acd.history.test.db.url}" src="test/org/sipfoundry/sipxconfig/acd/stats/history-seed.sql" />
# </target>

# <target name="reset-cdr-db">
#   <property name="cdr.test.db.url" value="jdbc:postgresql://localhost/${cdr.test.db}" />
#   <exec executable="dropdb" outputproperty="eatme">
#     <arg line="-U ${sipxconfig.db.user} ${cdr.test.db}" />
#   </exec>
#   <exec executable="createdb" outputproperty="eatme">
#     <arg line="-U ${sipxconfig.db.user} ${cdr.test.db}" />
#   </exec>
#   <pgsql url="${cdr.test.db.url}" src="${cdr.db.schema}" />
#   <pgsql url="${cdr.test.db.url}" src="test/org/sipfoundry/sipxconfig/cdr/cdr_seed.sql" />
# </target>


# <target name="dataset-db"
#         description="Generates the sipconfig-db.dtd which is used to validate datasets"
#         depends="compile.test, reset-db">
#   <java classname="org.sipfoundry.sipxconfig.GenerateDatabaseDtd" classpathref="test.path" fork="on">
#     <arg value="${test.classes.dir}" />
#   </java>
#   <echo level="info" message="Please remember to commit new sipxconfig-db.dtd file" />
# </target>
