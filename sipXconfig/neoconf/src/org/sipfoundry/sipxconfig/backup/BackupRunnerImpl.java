/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import org.sipfoundry.sipxconfig.common.SimpleCommandRunner;
import org.sipfoundry.sipxconfig.common.UserException;

public class BackupRunnerImpl implements BackupRunner {
    private SimpleCommandRunner m_actionRunner;
    private String m_backupScript;
    private int m_foregroundTimeout = 5000;

    @Override
    public boolean isInProgress() {
        return m_actionRunner != null && m_actionRunner.isInProgress();
    }

    public void setBackupScript(String backupScript) {
        m_backupScript = backupScript;
    }

    @Override
    public Map<String, List<String>> list(File plan) {
        if (!plan.exists()) {
            return Collections.emptyMap();
        }
        SimpleCommandRunner runner = new SimpleCommandRunner();
        String[] cmd = new String[] {
            m_backupScript, "--list", plan.getAbsolutePath()
        };
        if (!runner.run(cmd, m_foregroundTimeout)) {
            throw new RuntimeException("Timeout getting backup listing");
        } else if (runner.getExitCode() != 0) {
            throw new RuntimeException(runner.getStderr());
        }
        Map<String, List<String>> list = new TreeMap<String, List<String>>();
        String[] lines = runner.getStdout().split("\\n+");
        for (String line : lines) {
            String[] backup = line.split("\\s+");
            if (backup.length > 1) {
                String[] entries = new String[backup.length - 1];
                System.arraycopy(backup, 1, entries, 0, entries.length);
                list.put(backup[0], Arrays.asList(entries));
            }
        }
        return list;
    }

    SimpleCommandRunner obtainBackgroundRunner() {
        if (m_actionRunner == null) {
            m_actionRunner = new SimpleCommandRunner();
        } else if (m_actionRunner.isInProgress()) {
            throw new UserException("Operation still in progress");
        }
        return m_actionRunner;
    }

    @Override
    public boolean backup(File plan) {
        return op(plan, "--backup");
    }

    @Override
    public boolean restore(File plan) {
        return op(plan, "--restore");
    }

    boolean op(File plan, String operation) {
        SimpleCommandRunner runner = obtainBackgroundRunner();
        String[] cmd = new String[] {
            m_backupScript, operation, plan.getAbsolutePath()
        };
        return runner.run(cmd, m_foregroundTimeout);
    }
}
