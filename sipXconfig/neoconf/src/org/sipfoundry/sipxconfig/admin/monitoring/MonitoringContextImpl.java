/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.monitoring;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.service.SipxMrtgService;
import org.springframework.beans.factory.InitializingBean;
import org.springframework.beans.factory.annotation.Required;

public class MonitoringContextImpl implements MonitoringContext, InitializingBean {
    private static final Log LOG = LogFactory.getLog(MonitoringContextImpl.class);

    private LocationsManager m_locationsManager;

    private MRTGConfig m_templateMrtgConfig;

    private MRTGConfig m_mrtgConfig;

    private RRDToolGraphUpdater m_rrdToolGraphUpdater;

    private boolean m_enabled;

    private String m_communitySnmp;

    private SipxProcessContext m_processContext;

    private SipxMrtgService m_mrtgService;

    /**
     * set and load mrtg config file object
     */
    public void setMrtgConfig(MRTGConfig config) {
        m_mrtgConfig = config;
    }

    /**
     * set and load mrtg template config object
     */
    public void setMrtgTemplateConfig(MRTGConfig templateConfig) {
        m_templateMrtgConfig = templateConfig;
    }

    /**
     * Set the sipxReplicationContext needed for retrieving the locations within topology.xml The
     * locations will be displayed as available hosts to monitor in HA configuration
     */
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Required
    public void setProcessContext(SipxProcessContext processContext) {
        m_processContext = processContext;
    }

    @Required
    public void setMrtgService(SipxMrtgService mrtgService) {
        m_mrtgService = mrtgService;
    }


    /**
     * returns a list with all available hosts to monitor. In a HA configuration will return the
     * hosts configured in topology.xml file. In a non HA configuration will return 'localhost' as
     * the only available host to monitor
     *
     * Location.getSipDomain() should return the configured domains in a HA config. If sip_domain
     * element is null, we parse the replication_url and extract the host (replication url format
     * https://host.example.org:8091/cgi-bin/replication/replication.cgi)
     *
     */
    public List<String> getAvailableHosts() {
        List<String> hosts = new ArrayList<String>();
        Location[] locations = m_locationsManager.getLocations();
        for (Location location : locations) {
            if (location.isRegistered()) {
                String host = location.getFqdn();
                hosts.add(host);
            }
        }
        if (hosts.size() == 0) {
            hosts.add("localhost");
        }
        return hosts;
    }

    /**
     * Returns a list with the hosts that are already monitorized (configured in mrtg.cfg)
     */
    public List<String> getHosts() {
        loadMrtgConfig();
        return m_mrtgConfig.getHosts();
    }

    /**
     * creates the mrtg config object and parses the mrtg.cfg file (if mrtg.cfg file exists)
     *
     */
    private void loadMrtgConfig() {
        if ((new File(m_mrtgConfig.getFilename())).exists()) {
            try {
                m_mrtgConfig.parseConfig();
            } catch (Exception ex) {
                LOG.error("could not parse mrtg.cfg");
            }
        }
    }

    /**
     * returns a list with all the configured mrtg targets for the given host
     */
    public List<MRTGTarget> getTargetsForHost(String host) {
        List<MRTGTarget> targets = new ArrayList<MRTGTarget>();
        for (MRTGTarget target : m_mrtgConfig.getTargets()) {
            if (target.getHost().equalsIgnoreCase(host)) {
                targets.add(target);
            }
        }
        return targets;
    }

    /**
     * returns list with all available targets to monitor (from the template file)
     */
    public List<MRTGTarget> getTargetsFromTemplate() {
        return m_templateMrtgConfig.getTargets();
    }

    /**
     * creates the mrtg template object and parses the mrtg-t.cfg file
     *
     */
    private boolean loadTemplateMrtgConfig() {
        try {
            m_templateMrtgConfig.parseConfig();
        } catch (Exception ex) {
            LOG.error("could not parse mrtg template!");
            return false;
        }
        return true;
    }

    /**
     * returns list with all the available reports to monitor from template + summary report
     */
    public List<String> getReports(String host) {
        List<String> reports = new ArrayList<String>();
        List<MRTGTarget> targets = getTargetsForHost(host);
        for (MRTGTarget target : targets) {
            reports.add(target.getTitle());
        }
        return reports;
    }

    /**
     * returns the mrtgTarget object with the given target name for the given host
     */
    public MRTGTarget getMRTGTarget(String reportName, String host) {
        List<MRTGTarget> targets = getTargetsForHost(host);
        for (MRTGTarget target : targets) {
            if (target.getTitle().equalsIgnoreCase(reportName)) {
                return target;
            }
        }
        return null;
    }

    /**
     * use this one until sipXconfig will be able to configure SNMP agent, see XCF-1952
     *
     * @param host
     * @param selectedTargetNames
     */
    public void generateConfigFiles(String host, List<String> selectedTargetNames) {
        generateConfigFiles(host, m_communitySnmp, selectedTargetNames);
    }

    /**
     * writes the mrtg.cfg file with the selected targets for the provided host and snmp community
     * string Basically overwrites the mrtg.cfg file with the targets already configured at that
     * time plus new targets based on the provided host, community string and targets name
     *
     * The mrtg targets will be generated with ids like: templateTargetId_host.
     */
    private void generateConfigFiles(String host, String communityString, List<String> selectedTargetNames) {

        String hostTemplate = "$(host)";
        String snmpTemplate = "$(snmpString)";

        // read all the targets already configured for other hosts in mrtg.cfg file
        List<MRTGTarget> allTargets = new ArrayList<MRTGTarget>();
        List<String> hosts = getHosts();
        for (String hostFromFile : hosts) {
            if (!hostFromFile.equals(host)) {
                List<MRTGTarget> targetsForHost = getTargetsForHost(hostFromFile);
                for (MRTGTarget target : targetsForHost) {
                    allTargets.add(target);
                }
            }
        }

        // return the selected targets from template file and creates custom id and expression
        // for the given host / given snmpString
        List<MRTGTarget> templateTargets = getTargetsFromTemplate();
        List<MRTGTarget> selectedTargets = new ArrayList<MRTGTarget>();
        for (MRTGTarget templateTarget : templateTargets) {
            if (selectedTargetNames.contains(templateTarget.getTitle())) {
                // set id like templateTargetId_host
                templateTarget.setId(templateTarget.getId() + MonitoringUtil.UNDERSCORE + host.toLowerCase());
                // replace ${host} and ${snmpString} in template expression
                templateTarget.setExpression(templateTarget.getExpression().replace(hostTemplate, host).replace(
                        snmpTemplate, communityString));
                selectedTargets.add(templateTarget);
            }
        }

        allTargets.addAll(selectedTargets);

        // reload template mrtg config
        loadTemplateMrtgConfig();

        // write all targets to config file and restart mrtg
        writeTargetsToConfigFilesAndRestartMrtg(allTargets);
    }

    private void restartMrtg() {
        m_processContext.markServicesForRestart(Arrays.asList(m_mrtgService));
    }

    /**
     * generates the png files only if the rrd files were changed
     */
    public boolean updateGraphs(String host) {
        try {
            if (m_rrdToolGraphUpdater == null) {
                m_rrdToolGraphUpdater = new RRDToolGraphUpdater(m_templateMrtgConfig.getWorkingDir(),
                        m_templateMrtgConfig.getWorkingDir());
            }
            for (MRTGTarget target : getTargetsForHost(host)) {
                m_rrdToolGraphUpdater.updateAllGraphs(target);
            }
            return true;
        } catch (Exception e) {
            return false;
        }
    }

    /**
     * called after properties set, check if the targets from mrtg.cfg are having a corresponding
     * target in the template Covers the following cases: if one will manually delete a target
     * (that is already monitorized for a host) from the template, this target will be also
     * deleted from the mrtg.cfg when sipxconfig is restarted If one will manually add a new
     * target in the template then the target will appear as an available target to monitor after
     * sipxconfig is restarted.
     */
    private void intializeConfigFiles() {

        // if mrtg.cfg does not exists do nothing
        if (!(new File(m_mrtgConfig.getFilename())).exists()) {
            // Re-generate cfg file in case it was deleted before restart...
            writeTargetsToConfigFilesAndRestartMrtg(new ArrayList<MRTGTarget>());
            return;
        }

        // get all the target ids from template
        List<String> templateTargetIds = new ArrayList<String>();
        for (MRTGTarget target : m_templateMrtgConfig.getTargets()) {
            templateTargetIds.add(target.getId());
        }

        // already configured targets in mrtg.cfg file should have id like templateTargetId_host
        // checks if the target ids from mrtg.cfg starts with valid templateTargetId and writes
        // the mrtg.cfg
        // only with valid targets
        List<MRTGTarget> targetsToWrite = new ArrayList<MRTGTarget>();
        boolean shouldWriteConfigFile = false;

        for (MRTGTarget configuredTarget : m_mrtgConfig.getTargets()) {
            String configuredTargetId = configuredTarget.getId();
            int indexOfUnderscore = configuredTargetId.indexOf(MonitoringUtil.UNDERSCORE);

            if (indexOfUnderscore != -1) {
                if (!templateTargetIds.contains(configuredTargetId.substring(0, indexOfUnderscore))) {
                    // target to delete
                    shouldWriteConfigFile = true;
                } else {
                    targetsToWrite.add(configuredTarget);
                }
            } else {
                // if the target doesn't contain underscore => target to delete
                shouldWriteConfigFile = true;
            }
        }

        // writes targets to config file if needed and restarts mrtg
        if (shouldWriteConfigFile) {
            writeTargetsToConfigFilesAndRestartMrtg(targetsToWrite);
        }

    }

    /**
     * returns the mrtg working directory (/var/sipxdata/mrtg)
     */
    public String getMrtgWorkingDir() {
        return m_templateMrtgConfig.getWorkingDir();
    }

    private boolean writeTargetsToConfigFilesAndRestartMrtg(List<MRTGTarget> targets) {
        boolean toReturn = writeTargetsToConfigFiles(targets);
        restartMrtg();
        return toReturn;
    }

    /**
     * writes targets to the mrtg.cfg and restart mrtg
     */
    private boolean writeTargetsToConfigFiles(List<MRTGTarget> targets) {
        try {
            m_mrtgConfig.setRunAsDaemon(m_templateMrtgConfig.getRunAsDaemon());
            m_mrtgConfig.setNoDetach(m_templateMrtgConfig.getNoDetach());
            m_mrtgConfig.setInterval(m_templateMrtgConfig.getInterval());
            m_mrtgConfig.setIPV6(m_templateMrtgConfig.getIPV6());
            m_mrtgConfig.setWorkingDir(m_templateMrtgConfig.getWorkingDir());
            m_mrtgConfig.setThreshDir(m_templateMrtgConfig.getThreshDir());
            m_mrtgConfig.setMibs(m_templateMrtgConfig.getMibs());
            m_mrtgConfig.setTargets(targets);
            boolean writeResult = m_mrtgConfig.writeFile();

            // reload mrtg config object
            if (writeResult) {
                loadMrtgConfig();
            }

        } catch (Exception e) {
            LOG.error(e);
            return false;
        }

        return true;
    }

    /**
     * called by Spring framework after properties set initializes the mrtg config files
     */
    public void afterPropertiesSet() {
        if (!m_enabled) {
            return;
        }
        loadTemplateMrtgConfig();
        loadMrtgConfig();
        intializeConfigFiles();
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public void setCommunitySnmp(String communitySnmp) {
        m_communitySnmp = communitySnmp;
    }

}
