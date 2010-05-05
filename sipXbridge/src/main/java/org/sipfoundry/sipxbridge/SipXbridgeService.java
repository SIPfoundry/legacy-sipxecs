/**
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxbridge;

import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.log4j.SipFoundryLayout;
import org.sipfoundry.commons.sipXecsService.SipXecsService;

public class SipXbridgeService extends SipXecsService {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxbridge");

    public SipXbridgeService(String serviceName) {
        super(serviceName);

        // log4j configuration is mostly done by SipXecsService; just need to set our level
        String level = Gateway.getLogLevel().toString();
        if (level.equalsIgnoreCase("trace")) {
            level = "DEBUG";
        }
        setLogLevel(level);
    }

    // A config file has changed.
    // Child processes can implement this to handle the config changes as they wish.
    // Note that this method is called in the StdinListener thread, so the implementation must be threadsafe.
    public void resourceChanged(String fileType, String configFile) {
        LOG.debug("SipXbridge::resourceChanged " + configFile + " (type " + fileType + ")");

        // Reload the configuration and adjust the log level if necessary
        if (Gateway.getConfigFile().equals(configFile)) {
            ConfigurationParser parser = new ConfigurationParser();
            AccountManagerImpl newAccountManager = parser.createAccountManager(configFile);

            Level newLevel = SipFoundryLayout.mapSipFoundry2log4j(
                    newAccountManager.getBridgeConfiguration().getLogLevel());
            if (LOG.getLevel() != newLevel) {
                setLogLevel(newLevel.toString());
                Gateway.initializeLogging(newLevel.toString());
                ProtocolObjects.setLogLevel(newLevel.toString());
            }
        }
        else if (Gateway.getPeerIdentitiesFile().equals(configFile)) {
            Gateway.parsePeerIdentitiesFile();
        }
    }

    /// The child process must exit.
    public void shutdown() {
        // The script for sipxbridge sends a Stop command on shutdown.
        LOG.debug("sipxbridge waiting for stop command...");
    }

}
