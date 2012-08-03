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
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.UserException;

/**
 * Wraps functionality of cluster backup script and makes it available.
 */
public class BackupCommandRunner {
    private static final Log LOG = LogFactory.getLog(BackupCommandRunner.class);
    private String m_backupScript;
    private File m_plan;
    private boolean m_background;
    private String m_mode = "manual";

    public BackupCommandRunner(File plan, String backupScript) {
        m_backupScript = backupScript;
        m_plan = plan;
    }

    public String lastBackup() {
        List<String> backups = list();
        return backups.isEmpty() ? null : (String) backups.get(0);
    }

    public void restore(Collection<String> paths) {
        String[] args;
        if (paths == null || paths.isEmpty()) {
            // Already staged
            args = new String[2];
        } else {
            // Stages then restores
            args = new String[paths.size() + 2];
            System.arraycopy(paths.toArray(), 0, args, 2, paths.size());
        }
        args[0] = "--restore";
        args[1] = m_plan.getAbsolutePath();
        runCommand(args);
    }

    public void backup() {
        runCommand("--backup", m_plan.getAbsolutePath());
    }

    public String getBackupLink() {
        return StringUtils.chomp(runCommand("--link", m_plan.getAbsolutePath()));
    }

    String runCommand(String... command) {
        File listFile = null;
        Reader rdr = null;
        String commandLine = StringUtils.EMPTY;
        try {
            listFile = File.createTempFile("archive-command", ".tmp");
            String[] commandOut = new String[command.length + 5];
            System.arraycopy(command, 0, commandOut, 1, command.length);
            commandOut[0] = m_backupScript;
            commandOut[command.length + 1] = "--out";
            commandOut[command.length + 2] = listFile.getAbsolutePath();
            commandOut[command.length + 3] = "--mode"; // Relevant to few cmds, but harmless otherwise
            commandOut[command.length + 4] = m_mode;
            ProcessBuilder pb = new ProcessBuilder(commandOut);
            commandLine = StringUtils.join(pb.command(), ' ');
            LOG.info(commandLine);
            Process process = pb.start();
            if (m_background) {
                return StringUtils.EMPTY;
            }
            int code = process.waitFor();
            if (code != 0) {
                throw new UserException("&archive.command.failed", commandLine, code);
            }
            rdr = new FileReader(listFile);
            return IOUtils.toString(rdr);
        } catch (IOException e) {
            throw new UserException("&error.running.archive.command", commandLine);
        } catch (InterruptedException e) {
            throw new UserException("&timed.out.archive.command", commandLine);
        } finally {
            IOUtils.closeQuietly(rdr);
            if (listFile != null) {
                listFile.delete();
            }
        }
    }

    public List<String> list() {
        if (!m_plan.exists()) {
            return Collections.emptyList();
        }
        String lines = StringUtils.chomp(runCommand("--list", m_plan.getAbsolutePath()));
        return Arrays.asList(StringUtils.splitByWholeSeparator(lines, "\n"));
    }

    public boolean isBackground() {
        return m_background;
    }

    public void setBackground(boolean background) {
        m_background = background;
    }

    public String getMode() {
        return m_mode;
    }

    public void setMode(String mode) {
        m_mode = mode;
    }
}
