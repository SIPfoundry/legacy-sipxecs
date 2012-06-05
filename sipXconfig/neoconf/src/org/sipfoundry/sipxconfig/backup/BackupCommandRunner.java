/**
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
package org.sipfoundry.sipxconfig.backup;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.Reader;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;

/**
 * Wraps functionality of cluster backup script and makes it available.
 */
public class BackupCommandRunner {
    private String m_backupScript;
    private File m_plan;

    public BackupCommandRunner(File plan, String backupScript) {
        m_backupScript = backupScript;
        m_plan = plan;
    }

    public String lastBackup() {
        List<String> backups = list();
        return backups.isEmpty() ? null : (String) backups.get(backups.size() - 1);
    }

    public void backup() {
        runCommand(m_backupScript, "--backup", m_plan.getAbsolutePath());
    }

    void runCommand(String... command) {
        String commandLine = StringUtils.EMPTY;
        try {
            ProcessBuilder pb = new ProcessBuilder(command);
            commandLine = StringUtils.join(pb.command(), ' ');
            Process process = pb.start();
            int code = process.waitFor();
            if (code != 0) {
                String errorMsg = String.format("Backup command %s failed. Exit code: %d", commandLine, code);
                throw new RuntimeException(errorMsg);
            }
        } catch (IOException e) {
            String errorMsg = String.format("Error running backup command %s.", commandLine);
            throw new RuntimeException(errorMsg);
        } catch (InterruptedException e) {
            String errorMsg = String.format("Backup listing command timed out running command %s.", commandLine);
            throw new RuntimeException(errorMsg);
        }
    }

    @SuppressWarnings("unchecked")
    public List<String> list() {
        File listFile = null;
        Reader rdr = null;
        String cmd = StringUtils.EMPTY;
        try {
            listFile = File.createTempFile("backup-list", ".tmp");
            runCommand(m_backupScript, "--list", m_plan.getAbsolutePath(), "--out",  listFile.getAbsolutePath());
            rdr = new FileReader(listFile);
            return (List<String>) IOUtils.readLines(rdr);
        } catch (IOException e) {
            String errorMsg = String.format("Error during backup. %s", cmd);
            throw new RuntimeException(errorMsg);
        } finally {
            IOUtils.closeQuietly(rdr);
            if (listFile != null) {
                listFile.delete();
            }
        }
    }
}
