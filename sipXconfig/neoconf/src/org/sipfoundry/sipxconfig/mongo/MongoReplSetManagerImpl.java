/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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

import java.io.File;
import java.io.IOException;

import org.apache.commons.io.FileUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.common.BatchCommandRunner;
import org.sipfoundry.sipxconfig.common.CommandRunner;
import org.sipfoundry.sipxconfig.common.SimpleCommandRunner;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.region.Region;

public class MongoReplSetManagerImpl implements MongoReplSetManager {
    private static final Log LOG = LogFactory.getLog(MongoReplSetManagerImpl.class);
    private static final String BLANK_MODEL = "{ \"servers\" : [ ] , \"local\" : true , "
            + "\"arbiters\" : [ ] , \"replSet\" : \"sipxlocal\"}";
    private int m_timeout = 10000;
    private int m_backgroundTimeout = 120000; // can take a while for fresh mongo to init
    private String m_mongoStatusScript;
    private String m_mongoAnalyzerScript;
    private String m_mongoAdminScript;
    private ConfigManager m_configManager;
    private FeatureManager m_featureManager;
    private CommandRunner m_actionRunner;
    private Object m_lastErrorToken;
    private Region m_region;
    private LocationFeature m_dbFeature = MongoManager.FEATURE_ID;
    private LocationFeature m_arbFeature = MongoManager.ARBITER_FEATURE;
    private LocationFeature m_ldbFeature = MongoManager.LOCAL_FEATURE;
    private LocationFeature m_larbFeature = MongoManager.LOCAL_ARBITER_FEATURE;

    @Override
    public MongoMeta getMeta() {
        MongoMeta meta = new MongoMeta();
        StringBuilder cmd = new StringBuilder(m_mongoStatusScript).append(" --full");
        if (m_region != null) {
            appendModel(cmd);
        }
        String statusToken = run(new SimpleCommandRunner(), cmd.toString());
        meta.setStatusToken(statusToken);

        if (statusToken != null) {
            SimpleCommandRunner arunner = new SimpleCommandRunner();
            arunner.setStdin(statusToken);
            String analysisToken = run(arunner, m_mongoAnalyzerScript);
            meta.setAnalysisToken(analysisToken);
        }

        return meta;
    }

    void appendModel(StringBuilder cmd) {
        File modelFile = MongoConfig.getShardModelFile(m_configManager, m_region);
        // on very first db add, file may not exist
        if (!modelFile.exists()) {
            try {
                FileUtils.writeStringToFile(modelFile, BLANK_MODEL);
            } catch (IOException e) {
                throw new RuntimeException("Could not generate initial model file", e);
            }
        }
        cmd.append(" --model ");
        cmd.append(modelFile.getName());
    }

    @Override
    public String addFirstLocalDatabase(String hostPort) {
        checkInProgress();
        String fqdn = MongoNode.fqdn(hostPort);
        Location l = m_configManager.getLocationManager().getLocationByFqdn(fqdn);
        m_featureManager.enableLocationFeature(MongoManager.LOCAL_FEATURE, l, true);
        return run("adding local db " + hostPort, new ConfigCommandRunner(),
                createSimpleAction(fqdn, hostPort, "INITIALIZE", m_backgroundTimeout));
    }

    /**
     * Only to be used on last local database or arbiter, otherwise just removeDatabase
     */
    @Override
    public String removeLastLocalDatabase(String hostPort) {
        return removeLastLocal(hostPort, m_ldbFeature);
    }

    /**
     * Only to be used on last local database or arbiter, otherwise just removeDatabase
     */
    @Override
    public String removeLastLocalArbiter(String hostPort) {
        return removeLastLocal(hostPort, m_larbFeature);
    }

    public String removeLastLocal(String hostPort, LocationFeature f) {
        checkInProgress();
        String fqdn = MongoNode.fqdn(hostPort);
        Location l = m_configManager.getLocationManager().getLocationByFqdn(fqdn);
        m_featureManager.enableLocationFeature(f, l, false);
        return run("removing local db " + hostPort, new ConfigCommandRunner());
    }

    @Override
    public String addLocalDatabase(String primary, String hostPort) {
        return add(primary, hostPort, m_ldbFeature);
    }

    @Override
    public String addLocalArbiter(String primary, String hostPort) {
        return add(primary, hostPort, m_larbFeature);
    }

    @Override
    public String removeLocalDatabase(String primary, String hostPort) {
        return add(primary, hostPort, m_ldbFeature);
    }

    @Override
    public String removeLocalArbiter(String primary, String hostPort) {
        return add(primary, hostPort, m_larbFeature);
    }

    @Override
    public boolean isInProgress() {
        return m_actionRunner != null && m_actionRunner.isInProgress();
    }

    @Override
    public String addDatabase(String primary, String hostPort) {
        return add(primary, hostPort, m_dbFeature);
    }

    @Override
    public String addArbiter(String primary, String hostPort) {
        return add(primary, hostPort, m_arbFeature);
    }

    @Override
    public String removeDatabase(String primary, String hostPort) {
        return remove(primary, hostPort, m_dbFeature);
    }

    @Override
    public String removeArbiter(String primary, String hostPort) {
        return remove(primary, hostPort, m_arbFeature);
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

    String run(String msg, CommandRunner... runners) {
        BatchCommandRunner batch = new BatchCommandRunner(msg);
        batch.setBackgroundTimeout(m_backgroundTimeout);
        m_actionRunner = batch;
        for (CommandRunner runner : runners) {
            batch.add(runner);
        }
        boolean done = batch.run();
        return done ? batch.getStdout() : null;
    }

    /**
     * Nothing fancy because it would never run fast enough to return in the foreground timeout
     * and even if there were errors, they may not be related to what you're trying to do and
     * you'd likely have to ignore them anyway. The error still go into the job status table
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

    @Override
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
        if (m_region != null) {
            appendModel(cmd);
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

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    public Region getRegion() {
        return m_region;
    }

    public void setRegion(Region region) {
        m_region = region;
        if (m_region != null) {
            m_dbFeature = MongoManager.LOCAL_FEATURE;
            m_arbFeature = MongoManager.LOCAL_ARBITER_FEATURE;
        }
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    public void setMongoStatusScript(String mongoStatusScript) {
        m_mongoStatusScript = mongoStatusScript;
    }

    public void setMongoAnalyzerScript(String mongoAnalyzerScript) {
        m_mongoAnalyzerScript = mongoAnalyzerScript;
    }

    public void setMongoAdminScript(String mongoAdminScript) {
        m_mongoAdminScript = mongoAdminScript;
    }
}
