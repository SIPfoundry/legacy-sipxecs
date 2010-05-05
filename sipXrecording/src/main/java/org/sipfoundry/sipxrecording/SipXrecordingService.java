/**
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrecording;

import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.log4j.SipFoundryLayout;
import org.sipfoundry.commons.sipXecsService.SipXecsService;

public class SipXrecordingService extends SipXecsService {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxrecording");

    public SipXrecordingService(String serviceName) {
        super(serviceName);
    }

    // A config file has changed.
    // Child processes can implement this to handle the config changes as they wish.
    // Note that this method is called in the StdinListener thread, so the implementation must be threadsafe.
    public void resourceChanged(String fileType, String configFile) {
        LOG.debug("SipXrecording::resourceChanged " + configFile + " (type " + fileType + ")");

        // Reload the configuration and adjust the log level if necessary
        if (RecordingConfiguration.getConfigFile().equals(configFile)) {
            RecordingConfiguration config = RecordingConfiguration.get();
            Level newLevel = SipFoundryLayout.mapSipFoundry2log4j(
                    config.getLogLevel());
            if (LOG.getLevel() != newLevel) {
                setLogLevel(newLevel.toString());
            }
        }
    }

}
