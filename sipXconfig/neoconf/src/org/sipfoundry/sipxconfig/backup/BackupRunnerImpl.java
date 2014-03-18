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
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.alarm.AlarmDefinition;
import org.sipfoundry.sipxconfig.alarm.AlarmProvider;
import org.sipfoundry.sipxconfig.alarm.AlarmServerManager;
import org.sipfoundry.sipxconfig.common.SimpleCommandRunner;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.util.StringUtils;

public class BackupRunnerImpl implements BackupRunner, AlarmProvider {
    private SimpleCommandRunner m_actionRunner;
    private String m_backupScript;
    private int m_defaultForegroundTimeout = 5000;
    private int m_defaultBackgroundTimeout = 300000;
    private JobContext m_jobContext;

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
        SimpleCommandRunner runner = new SimpleCommandRunner(m_jobContext);
        runner.setJobName("List Backups");
        String[] cmd = new String[] {
            m_backupScript, "--list", plan.getAbsolutePath()
        };
        op(runner, cmd, 15000, 0);
        Map<String, List<String>> list = new LinkedHashMap<String, List<String>>();
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
            m_actionRunner = new SimpleCommandRunner(m_jobContext);
        } else if (m_actionRunner.isInProgress()) {
            throw new UserException("Operation still in progress");
        }
        return m_actionRunner;
    }

    @Override
    public void backup(File plan) {
        op(plan, "--backup", m_defaultForegroundTimeout, m_defaultBackgroundTimeout, "Backup");
    }

    @Override
    public void restore(File plan, Collection<String> selections) {
        Collection<String> params = new ArrayList<String>();
        params.add(plan.getAbsolutePath());
        params.addAll(selections);
        String stringParams = StringUtils.collectionToCommaDelimitedString(params);
        op("--restore", stringParams, m_defaultForegroundTimeout, m_defaultBackgroundTimeout, "Restore");
    }

    void op(File plan, String operation, int foregroundTimeout, int backgroundTimeout, String jobName) {
        SimpleCommandRunner runner = obtainBackgroundRunner();
        runner.setJobName(jobName);
        String[] cmd = new String[] {
            m_backupScript, operation, plan.getAbsolutePath()
        };
        op(runner, cmd, foregroundTimeout, backgroundTimeout);
    }

    /**
     * Ruby script accepts parameters like this: a,b,c
     * @param plan = backup plan config file
     * @param operation = --stage or --restore etc
     * @param params = /backup.yaml,201401301800/configuration.tar.gz, 201401301800/voicemail.tar.gz
     * @return
     */
    void op(String operation, String params, int foregroundTimeout, int backgroundTimeout, String jobName) {
        SimpleCommandRunner runner = obtainBackgroundRunner();
        runner.setJobName(jobName);
        String[] cmd = new String[] {
            m_backupScript, operation, params
        };
        op(runner, cmd, foregroundTimeout, backgroundTimeout);
    }
    /**
     * Generic command call with exception handling. Any ruby script exception is kept in runner.getStderr()
     * @param runner
     * @param cmd
     * @return
     */
    private void op(SimpleCommandRunner runner, String[] cmd, int foregroundTimeout, int backgroundTimeout) {
        if (!runner.run(cmd, foregroundTimeout, backgroundTimeout)) {
            throw new TimeoutException();
        } else if (runner.getExitCode() != null && runner.getExitCode() != 0) {
            throw new StdErrException(runner.getStderr());
        }
    }

    @Required
    public void setJobContext(JobContext jobContext) {
        m_jobContext = jobContext;
    }

    @Override
    public Collection<AlarmDefinition> getAvailableAlarms(AlarmServerManager manager) {
        return Collections.singleton(BACKUP_FAILED);
    }

    public static class TimeoutException extends RuntimeException {
        public TimeoutException() {
            super("Timeout running operation ");
        }
    }

    public static class StdErrException extends RuntimeException {
        public StdErrException(String errMessage) {
            super(errMessage);
        }
    }

}
