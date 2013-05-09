include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(bin_SCRIPTS:=.in) \
	$(initd_SCRIPTS:=.in)

bin_SCRIPTS = \
	mongodb-status \
	mongodb-analyzer \
	mongodb-admin

mongodb_TESTS = \
	bad-replset \
	one-node-uninit \
	two-node-missing-arbiter \
	two-node-uninitialized \
	two-node-one-initializing \
	two-node-one-removed \
	two-node-removed-arbiter \
	two-node-orphaned-arbiter \
	two-node-db-unaware-of-arbiter \
	two-node-bad-config-ver \
	three-node-unavailable-server \
	three-node-unhealthy-server \
	three-node-missing-arbiter-and-database \
	three-node-missing-database \
	three-node-healthy

mongodb_admin_primary_COMMANDS = \
	"--host_port swift.hubler.us:27017 RESET_LOCAL" \
	"--host_port swift.hubler.us:27017 OBLITERATE" \
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
	  expected="$(srcdir)/test_data/$$t.status.json"; \
	  input="$(srcdir)/test_data/$$t.json"; \
	  echo "EXPECTED $$expected"; \
	  echo "$(srcdir)/mongodb-status.in --parse < $$input"; \
	  $(srcdir)/mongodb-status.in --parse < $$input  > actual; \
	  diff -u $$expected actual || exit 1; \
	done

check-analyzer:
	@echo "====mongodb-analysis tests===="
	@for t in $(mongodb_TESTS); do \
	  expected="$(srcdir)/test_data/$$t.analysis.json"; \
	  input="$(srcdir)/test_data/$$t.status.json"; \
	  echo "EXPECTED: $$expected"; \
	  echo "$(srcdir)/mongodb-analyzer.in < $$input"; \
	  $(srcdir)/mongodb-analyzer.in < $$input > actual; \
	  diff -u $$expected actual  || exit 1; \
	done

check-admin:
	@echo "====mongodb-admin tests===="
	@for c in $(mongodb_admin_primary_COMMANDS); do \
	  expected=$(srcdir)/test_data/`echo -n $$c | sed -e 's/[^a-zA-Z0-9]/_/g'`.primary.dryrun; \
	  cmd="$(srcdir)/mongodb-admin.in --dryrun --model $(srcdir)/test_data/admin.model.json --primary swift.hubler.us:27017 $$c"; \
	  echo "$$cmd"; \
	  echo "EXPECTED : $$expected"; \
	  $$cmd > actual; \
	  diff -u $$expected actual || exit 1; \
	done
	@for c in $(mongodb_admin_secondary_COMMANDS); do \
	  expected=$(srcdir)/test_data/`echo -n $$c | sed -e 's/[^a-zA-Z0-9]/_/g'`.secondary.dryrun; \
	  cmd="$(srcdir)/mongodb-admin.in --host_port swift.hubler.us:27019 --dryrun --model $(srcdir)/test_data/admin.model.json $$c"; \
	  echo "$$cmd"; \
	  echo "EXPECTED : $$expected"; \
	  $$cmd > actual; \
	  diff -u $$expected actual || exit 1; \
	done
	@for c in $(mongodb_admin_arbiter_COMMANDS); do \
	  expected=$(srcdir)/test_data/`echo -n $$c | sed -e 's/[^a-zA-Z0-9]/_/g'`.arbiter.dryrun; \
	  cmd="$(srcdir)/mongodb-admin.in --host_port swift.hubler.us:27018 --dryrun --model $(srcdir)/test_data/admin.model.json $$c"; \
	  echo "$$cmd"; \
	  echo "EXPECTED : $$expected"; \
	  $$cmd > actual; \
	  diff -u $$expected actual || exit 1; \
	done

initddir = @SIPX_SERVICEDIR@
initd_SCRIPTS = \
	mongod-arbiter

$(initd_SCRIPTS) $(bin_SCRIPTS) : % : %.in
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = $(initd_SCRIPTS) $(bin_SCRIPTS)
