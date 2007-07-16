/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * Interface to command line restore utility
 */

public class Restore {

    public static final String BACKUP_CONFIGS = "configuration.tar.gz";

    public static final String BACKUP_MAILSTORE = "voicemail.tar.gz";

    private static final Log LOG = LogFactory.getLog(Restore.class);

    private static final String ERROR = "Errors when executing restore script: {0}";

    private static final String RESTORE_BINARY = "sipx-sudo-restore";

    private String m_binDirectory;

    public boolean perform(List<BackupBean> backups) {
        try {
            Runtime runtime = Runtime.getRuntime();
            runtime.exec(getCmdLine(backups));

        } catch (IOException e) {
            LOG.error(ERROR);
            return false;
        }
        return true;
    }

    String[] getCmdLine(List<BackupBean> backups) {

        File executable = new File(getBinDirectory(), RESTORE_BINARY);
        List<String> cmds = new ArrayList<String>();
        cmds.add(executable.getAbsolutePath());

        for (int i = 0; i < backups.size(); i++) {
            if (backups.get(i).getName().equalsIgnoreCase("Configuration")) {
                cmds.add("-c");
                cmds.add(backups.get(i).getPath());
            } else if (backups.get(i).getName().equalsIgnoreCase("Voicemail")) {
                cmds.add("-v");
                cmds.add(backups.get(i).getPath());
            } else {
                LOG.error("getCmdLine unknown archive type: '" + backups.get(i).getName() + "'.");
            }
        }
        cmds.add("--non-interactive");
        return cmds.toArray(new String[cmds.size()]);
    }

    public String getBinDirectory() {
        return m_binDirectory;
    }

    public void setBinDirectory(String binDirectory) {
        m_binDirectory = binDirectory;
    }
}
