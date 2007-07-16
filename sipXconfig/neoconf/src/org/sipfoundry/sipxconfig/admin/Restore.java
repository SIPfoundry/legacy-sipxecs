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

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * Interface to command line restore utility
 */

public class Restore {
    private static final Log LOG = LogFactory.getLog(Restore.class);

    private static final String ERROR = "Errors when executing restore script: %s";

    private static final String RESTORE_BINARY = "sipx-sudo-restore";

    private String m_binDirectory;

    public boolean perform(List<BackupBean> backups) {
        String[] cmdLine = getCmdLine(backups);
        try {
            Runtime runtime = Runtime.getRuntime();
            runtime.exec(cmdLine);
            return true;
        } catch (IOException e) {
            LOG.error(String.format(ERROR, StringUtils.join(cmdLine, " ")));
            return false;
        }
    }

    String[] getCmdLine(List<BackupBean> backups) {
        File executable = new File(getBinDirectory(), RESTORE_BINARY);
        List<String> cmds = new ArrayList<String>();
        cmds.add(executable.getAbsolutePath());

        for (BackupBean backup : backups) {
            cmds.add(backup.getType().getOption());
            cmds.add(backup.getPath());
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
