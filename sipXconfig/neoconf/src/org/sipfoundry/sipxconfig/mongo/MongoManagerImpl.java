/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.mongo;


import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.alarm.AlarmDefinition;
import org.sipfoundry.sipxconfig.alarm.AlarmProvider;
import org.sipfoundry.sipxconfig.alarm.AlarmServerManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.common.BatchCommandRunner;
import org.sipfoundry.sipxconfig.common.CommandRunner;
import org.sipfoundry.sipxconfig.common.SimpleCommandRunner;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.InvalidChange;
import org.sipfoundry.sipxconfig.feature.InvalidChangeException;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.firewall.DefaultFirewallRule;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;
import org.sipfoundry.sipxconfig.firewall.FirewallProvider;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;
import org.springframework.beans.factory.annotation.Required;

public class MongoManagerImpl implements AddressProvider, FeatureProvider, MongoManager, ProcessProvider,
        SetupListener, FirewallProvider, AlarmProvider {
    private static final Log LOG = LogFactory.getLog(MongoManagerImpl.class);
    private BeanWithSettingsDao<MongoSettings> m_settingsDao;
    private int m_timeout = 10000;
    private int m_backgroundTimeout = 120000; // can take a while for fresh mongo to init
    private String m_mongoStatusScript;
    private String m_mongoAnalyzerScript;
    private String m_mongoAdminScript;
    private CommandRunner m_actionRunner;
    private Object m_lastErrorToken;
    private ConfigManager m_configManager;
    private FeatureManager m_featureManager;

    public MongoMeta getMeta() {
        SimpleCommandRunner srunner = new SimpleCommandRunner();
        String statusToken = run(srunner, m_mongoStatusScript + " --full");
        MongoMeta meta = new MongoMeta();
        meta.setStatusToken(statusToken);

        if (statusToken != null) {
            SimpleCommandRunner arunner = new SimpleCommandRunner();
            arunner.setStdin(statusToken);
            String analysisToken = run(arunner, m_mongoAnalyzerScript);
            meta.setAnalysisToken(analysisToken);
        }

        return meta;
    }

    public boolean isInProgress() {
        return m_actionRunner != null && m_actionRunner.isInProgress();
    }

    @Override
    public String getLastConfigError() {
        if (m_actionRunner != null && !m_actionRunner.isInProgress()) {
            if (m_actionRunner.getExitCode() != null && m_actionRunner.getExitCode() != 0) {
                // use token to recall if we've already returned the error once
                if (m_lastErrorToken != m_actionRunner) {
                    m_lastErrorToken = m_actionRunner;
                    return m_actionRunner.getStderr();
                }
            }
        }
        return null;
    }

    public String addDatabase(String primary, String hostPort) {
        return add(primary, hostPort, MongoManager.FEATURE_ID);
    }

    public String addArbiter(String primary, String hostPort) {
        return add(primary, hostPort, MongoManager.ARBITER_FEATURE);
    }

    @Override
    public String removeDatabase(String primary, String hostPort) {
        return remove(primary, hostPort, MongoManager.FEATURE_ID);
    }

    @Override
    public String removeArbiter(String primary, String hostPort) {
        return remove(primary, hostPort, MongoManager.ARBITER_FEATURE);
    }

    String add(String primary, String hostPort, LocationFeature feature) {
        checkInProgress();
        String host = MongoNode.fqdn(hostPort);
        Location l = m_configManager.getLocationManager().getLocationByFqdn(host);
        m_featureManager.enableLocationFeature(feature, l, true);

        BatchCommandRunner batch = new BatchCommandRunner("adding " + hostPort);
        batch.setBackgroundTimeout(m_backgroundTimeout);
        m_actionRunner = batch;
        batch.add(new ConfigCommandRunner());
        batch.add(createSimpleAction(primary, hostPort, "ADD", m_backgroundTimeout));
        boolean done = batch.run();
        return done ? batch.getStdout() : null;
    }

    String remove(String primary, String hostPort, LocationFeature feature) {
        checkInProgress();
        String host = MongoNode.fqdn(hostPort);
        Location l = m_configManager.getLocationManager().getLocationByFqdn(host);
        m_featureManager.enableLocationFeature(feature, l, false);

        BatchCommandRunner batch = new BatchCommandRunner("removing " + hostPort);
        batch.setBackgroundTimeout(m_backgroundTimeout);
        m_actionRunner = batch;
        batch.add(createSimpleAction(primary, hostPort, "REMOVE", m_backgroundTimeout));
        batch.add(new ConfigCommandRunner());
        boolean done = batch.run();
        return done ? batch.getStdout() : null;
    }

    /**
     * Nothing fancy because it would never run fast enough to return in the foreground timeout
     * and even if there were errors, they may not be related to what you're trying to
     * do and you'd likely have to ignore them anyway.  The error still go into the job
     * status table
     */
    class ConfigCommandRunner implements CommandRunner {
        private boolean m_inProgress;

        @Override
        public boolean run() {
            m_inProgress = true;
            m_configManager.run();
            m_inProgress = false;
            synchronized (this) {
                ConfigCommandRunner.this.notifyAll();
            }
            return true;
        }

        @Override
        public boolean isInProgress() {
            return m_inProgress;
        }

        @Override
        public String getStdout() {
            return null;
        }

        @Override
        public String getStderr() {
            return null;
        }

        @Override
        public Integer getExitCode() {
            return 0;
        }
    };

    public void checkInProgress() {
        if (isInProgress()) {
            throw new UserException("Operation still in progress");
        }
    }

    public String takeAction(String primary, String hostPort, String action) {
        checkInProgress();

        SimpleCommandRunner runner = createSimpleAction(primary, hostPort, action, 0);
        m_actionRunner = runner;
        boolean done = m_actionRunner.run();
        if (done) {
            if (m_actionRunner.getExitCode() != 0) {
                throw new UserException("Command did not complete properly. " + m_actionRunner.getStderr());
            }
        }

        return done ? m_actionRunner.getStdout() : null;
    }

    SimpleCommandRunner createSimpleAction(String primary, String hostPort, String action, int background) {
        SimpleCommandRunner runner = new SimpleCommandRunner();
        String fqdn = MongoNode.fqdn(hostPort);
        String remote = m_configManager.getRemoteCommand(fqdn);
        StringBuilder cmd = new StringBuilder(remote);
        cmd.append(' ').append(m_mongoAdminScript);
        cmd.append(" --host_port ").append(hostPort);
        if (primary != null) {
            cmd.append(" --primary ").append(primary);
        }
        cmd.append(' ').append(action);
        String command = cmd.toString();
        runner.setRunParameters(command, m_timeout, background);
        return runner;
    }

    String runBackgroundOk(SimpleCommandRunner runner, String cmd) {
        if (!runner.run(SimpleCommandRunner.split(cmd), m_timeout, m_backgroundTimeout)) {
            return null;
        }
        return getOutput(cmd, runner.getStdin(), runner);
    }

    String run(SimpleCommandRunner runner, String cmd) {
        if (!runner.run(SimpleCommandRunner.split(cmd), m_timeout)) {
            throw new UserException(cmd + " did not complete in time");
        }
        return getOutput(cmd, runner.getStdin(), runner);
    }

    String getOutput(String cmd, String in, CommandRunner runner) {
        if (0 != runner.getExitCode()) {
            String err = runner.getStderr();
            if (err != null) {
                err = cmd + " had exit code " + runner.getExitCode();
            }
            if (in != null) {
                LOG.error(in);
            }
            throw new UserException(err);
        }

        return runner.getStdout();
    }

    public void setBackgroundTimeout(int backgroundTimeout) {
        m_backgroundTimeout = backgroundTimeout;
    }

    public void setMongoAdminScript(String mongoAdminScript) {
        m_mongoAdminScript = mongoAdminScript;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    public ConfigManager getConfigManager() {
        return m_configManager;
    }

    public void setTimeout(int timeout) {
        m_timeout = timeout;
    }

    @Required
    public void setMongoStatusScript(String mongoStatusScript) {
        m_mongoStatusScript = mongoStatusScript;
    }

    @Required
    public void setMongoAnalyzerScript(String mongoAnalyzerScript) {
        m_mongoAnalyzerScript = mongoAnalyzerScript;
    }

    @Override
    public MongoSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    @Override
    public void saveSettings(MongoSettings settings) {
        m_settingsDao.upsert(settings);
    }

    @Override
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        return Arrays.asList(new DefaultFirewallRule(ADDRESS_ID), new DefaultFirewallRule(ARBITOR_ADDRESS_ID));
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Location requester) {
        if (!type.equalsAnyOf(ADDRESS_ID, ARBITOR_ADDRESS_ID)) {
            return null;
        }

        LocationFeature feature = (type == ADDRESS_ID ? FEATURE_ID : ARBITER_FEATURE);
        Collection<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(feature);
        Collection<Address> addresses = Location.toAddresses(type, locations);
        LOG.debug("Found " + locations.size() + " locations: " + addresses);
        return addresses;
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager) {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l) {
        if (l.isPrimary()) {
            // we show arbiter as an option even though there are many situations where
            // it would not make sense, but there are situations where you have to show it
            // in case admins needs to disable it. e.g admin deletes 2 of 3 servers and
            // then needs to disable arbiter on primary.
            return Collections.singleton(ARBITER_FEATURE);
        }
        return Arrays.asList(FEATURE_ID, ARBITER_FEATURE);
    }

    public void setSettingsDao(BeanWithSettingsDao<MongoSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        Collection<ProcessDefinition> procs = new ArrayList<ProcessDefinition>(2);
        if (manager.getFeatureManager().isFeatureEnabled(FEATURE_ID, location) || location.isPrimary()) {
            procs.add(ProcessDefinition.sysvByRegex("mongod", ".*/mongod.*-f.*/mongodb{0,1}.conf", true));
        }
        if (manager.getFeatureManager().isFeatureEnabled(ARBITER_FEATURE, location)) {
            ProcessDefinition def = ProcessDefinition.sipxByRegex("mongod-arbiter",
                    ".*/mongod.*-f.*/mongod-arbiter.conf");
            def.setRestartClass("restart_mongo_arbiter");
            procs.add(def);
        }
        return procs;
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        // we do not list features in features list because enabling/disabling dbs
        // and arbiters required a dedicated ui.
    }

    @Override
    public boolean setup(SetupManager manager) {
        if (!manager.isTrue(FEATURE_ID.getId())) {
            Location primary = manager.getConfigManager().getLocationManager().getPrimaryLocation();
            manager.getFeatureManager().enableLocationFeature(FEATURE_ID, primary, true);
            manager.setTrue(FEATURE_ID.getId());
        }
        return true;
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        FeatureChangeRequest request = validator.getRequest();
        if (!request.hasChanged(FEATURE_ID)) {
            return;
        }

        Collection<Location> mongos = validator.getLocationsForEnabledFeature(FEATURE_ID);
        if (mongos.size() == 0) {
            InvalidChangeException err = new InvalidChangeException("&error.noMongos");
            InvalidChange needArbiter = new InvalidChange(FEATURE_ID, err);
            validator.getInvalidChanges().add(needArbiter);
        } else {
            boolean includesPrimary = false;
            for (Location l : mongos) {
                includesPrimary = l.isPrimary();
                if (includesPrimary) {
                    break;
                }
            }
            if (!includesPrimary) {
                InvalidChangeException err = new InvalidChangeException("&error.mongoOnPrimaryRequired");
                InvalidChange removeArbiter = new InvalidChange(FEATURE_ID, err);
                validator.getInvalidChanges().add(removeArbiter);
            }
        }
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
    }

    @Override
    public Collection<AlarmDefinition> getAvailableAlarms(AlarmServerManager manager) {
        if (!manager.getFeatureManager().isFeatureEnabled(MongoManager.FEATURE_ID)
                || !manager.getFeatureManager().isFeatureEnabled(MongoManager.FEATURE_ID)) {
            return null;
        }
        Collection<AlarmDefinition> defs = Arrays.asList(new AlarmDefinition[] {
            MONGO_FATAL_REPLICATION_STOP, MONGO_FAILED_ELECTION, MONGO_MEMBER_DOWN, MONGO_NODE_STATE_CHANGED,
            MONGO_CANNOT_SEE_MAJORITY
        });
        return defs;
    }

    @Override
    public boolean isMisconfigured() {
        // TODO Auto-generated method stub
        return false;
    }
}
