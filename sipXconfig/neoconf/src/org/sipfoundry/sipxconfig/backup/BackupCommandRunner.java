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

import static java.lang.String.format;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.Reader;
import java.util.List;

import org.apache.commons.io.IOUtils;

/**
 * Wraps functionality of cluster backup script and makes it available.
 */
public class BackupCommandRunner {
    private String m_backupScript;
    private String m_planConfigId;

    public BackupCommandRunner(BackupType type, String backupScript) {
        this(type.toString(), backupScript);
    }

    public BackupCommandRunner(String planConfigId, String backupScript) {
        m_backupScript = backupScript;
        m_planConfigId = planConfigId;
    }

    public String lastBackup() {
        List<String> backups = list();
        return backups.isEmpty() ? null : (String) backups.get(backups.size() - 1);
    }

    public void backup() {
        runCommand(format("%s --backup --plan %s", m_backupScript, m_planConfigId));
    }

    void runCommand(String command) {
        try {
            ProcessBuilder pb = new ProcessBuilder(command);
            Process process = pb.start();
            int code = process.waitFor();
            if (code != 0) {
                String errorMsg = String.format("Backup command %s failed. Exit code: %d", command, code);
                throw new RuntimeException(errorMsg);
            }
        } catch (IOException e) {
            String errorMsg = String.format("Error running backup command %s.", command);
            throw new RuntimeException(errorMsg);
        } catch (InterruptedException e) {
            String errorMsg = String.format("Backup listing command timed out running command %s.", command);
            throw new RuntimeException(errorMsg);
        }
    }

    @SuppressWarnings("unchecked")
    public List<String> list() {
        File listFile = null;
        Reader rdr = null;
        String cmd = "";
        try {
            listFile = File.createTempFile("backup-list", ".tmp");
            cmd = format("%s --list --plan %s --out %s", m_backupScript, m_planConfigId, listFile.getAbsolutePath());
            runCommand(cmd);
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
