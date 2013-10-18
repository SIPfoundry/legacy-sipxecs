include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/sipxconfig.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(bin_SCRIPTS:=.in) \
	$(initd_SCRIPTS:=.in) \
	tunnel.in \
	$(libexec_SCRIPTS:=.in) \
	mongo-test-data

initddir = @SIPX_SERVICEDIR@
initd_SCRIPTS = \
	sipxconfig \
	mongod-arbiter \
	mongo-local \
	mongo-local-arbiter \
	mongodb-sync

tunnel_daemondir = @SIPX_SERVICEDIR@
tunnel_daemon_SCRIPTS = \
	tunnel-server \
	tunnel-client

bin_SCRIPTS = \
	sipxconfig-archive \
	sipxconfig_archive.rb \
	mongodb-status \
	mongodb-analyzer \
	mongodb-admin \
	sipxtrap

libexecdir = $(SIPX_LIBEXECDIR)
libexec_SCRIPTS = \
	sipx-dns-validator \
	sipx-erb \
	mongodb-initial-sync \
	mongodb-move-sync \
	mongodb-oplog-sync \
	snmptrap-email-handler \
	snmp-fix-process \
	snmp-fix-process-cron

tunnel-% : MODE = $(word 2,$(subst -, ,$@))
$(tunnel_daemon_SCRIPTS) : tunnel.in Makefile
	@$(call SearchAndReplace,$<,$@,MODE)

$(initd_SCRIPTS) $(bin_SCRIPTS) $(libexec_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@,$(EXTRA_REPLACE))

mongodb_TESTS = \
	bad-replset \
	one-node-uninit \
	one-node-missing-meta \
	two-node-missing-arbiter \
	two-node-uninitialized \
	two-node-one-initializing \
	two-node-one-removed \
	two-node-removed-arbiter \
	two-node-orphaned-arbiter \
	two-node-db-unaware-of-arbiter \
	two-node-bad-config-ver \
	two-node-multiple-primaries \
	three-node-unavailable-server \
	three-node-unhealthy-server \
	three-node-missing-arbiter-and-database \
	three-node-missing-database \
	three-node-healthy

mongodb_admin_primary_COMMANDS = \
	"--host_port swift.hubler.us:27017 RESET_LOCAL" \
	"--host_port swift.hubler.us:27017 OBLITERATE" \
	"--host_port swift.hubler.us:27017 FORCE_SECONDARY" \
	"--host_port swift.hubler.us:27017 REPAIR" \
	"--host_port swift.hubler.us:27018 RENAME new_name:27018" \
	"--host_port swift.hubler.us:27017 INITIALIZE" \
	"--host_port swift.hubler.us:27018 ADD" \
	"--host_port swift.hubler.us:27019 ADD"

mongodb_admin_secondary_COMMANDS = \
	RESET_LOCAL \
	OBLITERATE

mongodb_admin_arbiter_COMMANDS = \
	RESET_LOCAL \
	OBLITERATE

check : check-status check-analyzer check-admin;

check-status:
	@echo "====mongodb-status tests===="
	@for t in $(mongodb_TESTS); do \
	  expected="$(srcdir)/mongo-test-data/$$t.status.json"; \
	  input="$(srcdir)/mongo-test-data/$$t.json"; \
	  echo "EXPECTED $$expected"; \
	  echo "$(srcdir)/mongodb-status.in --full --parse < $$input"; \
	  $(srcdir)/mongodb-status.in --full --parse < $$input  > actual; \
	  diff -u $$expected actual || exit 1; \
	done

check-analyzer:
	@echo "====mongodb-analysis tests===="
	@for t in $(mongodb_TESTS); do \
	  expected="$(srcdir)/mongo-test-data/$$t.analysis.json"; \
	  input="$(srcdir)/mongo-test-data/$$t.status.json"; \
	  echo "EXPECTED: $$expected"; \
	  echo "$(srcdir)/mongodb-analyzer.in < $$input"; \
	  $(srcdir)/mongodb-analyzer.in < $$input > actual; \
	  diff -u $$expected actual  || exit 1; \
	done

check-admin:
	@echo "====mongodb-admin tests===="
	@for c in $(mongodb_admin_primary_COMMANDS); do \
	  expected=$(srcdir)/mongo-test-data/`echo -n $$c | sed -e 's/[^a-zA-Z0-9]/_/g'`.primary.dryrun; \
	  cmd="$(srcdir)/mongodb-admin.in --dryrun --model $(srcdir)/mongo-test-data/admin.model.json --primary swift.hubler.us:27017 $$c"; \
	  echo "$$cmd"; \
	  echo "EXPECTED : $$expected"; \
	  $$cmd > actual; \
	  diff -u $$expected actual || exit 1; \
	done
	@for c in $(mongodb_admin_secondary_COMMANDS); do \
	  expected=$(srcdir)/mongo-test-data/`echo -n $$c | sed -e 's/[^a-zA-Z0-9]/_/g'`.secondary.dryrun; \
	  cmd="$(srcdir)/mongodb-admin.in --host_port swift.hubler.us:27019 --dryrun --model $(srcdir)/mongo-test-data/admin.model.json $$c"; \
	  echo "$$cmd"; \
	  echo "EXPECTED : $$expected"; \
	  $$cmd > actual; \
	  diff -u $$expected actual || exit 1; \
	done
	@for c in $(mongodb_admin_arbiter_COMMANDS); do \
	  expected=$(srcdir)/mongo-test-data/`echo -n $$c | sed -e 's/[^a-zA-Z0-9]/_/g'`.arbiter.dryrun; \
	  cmd="$(srcdir)/mongodb-admin.in --host_port swift.hubler.us:27018 --dryrun --model $(srcdir)/mongo-test-data/admin.model.json $$c"; \
	  echo "$$cmd"; \
	  echo "EXPECTED : $$expected"; \
	  $$cmd > actual; \
	  diff -u $$expected actual || exit 1; \
	done
	$(srcdir)/mongodb-admin.in --host_port swift.hubler.us:27019 --dryrun --primary swift.hubler.us:27017 --model $(srcdir)/mongo-test-data/SET_MEMBER_META-tags.model.json SET_MEMBER_META > actual
	diff -u $(srcdir)/mongo-test-data/SET_MEMBER_META-tags.dryrun actual

CLEANFILES = $(bin_SCRIPTS) $(initd_SCRIPTS) $(libexec_SCRIPTS) $(tunnel_daemon_SCRIPTS)
