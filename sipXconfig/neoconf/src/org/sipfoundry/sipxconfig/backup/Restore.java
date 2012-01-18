/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.backup;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.WaitingListener;
import org.sipfoundry.sipxconfig.vm.MailboxManager;

/**
 * Interface to command line restore utility
 */
public class Restore implements Serializable, WaitingListener {
    private static final Log LOG = LogFactory.getLog(Restore.class);

    private static final String ERROR = "Errors when executing restore script: %s";

    private static final String RESTORE_BINARY = "sipx-sudo-restore";

    private static final String SPACE = " ";

    private static final int INCOMPATIBLE_VERSIONS = 5;
    private static final int INVALID_CONFIGURATION_ARCHIVE = 7;
    private static final int INVALID_VOICEMAIL_ARCHIVE = 8;

    private static final String RESTORE_LOG = "sipx-restore.log";

    private String m_binDirectory;
    private String m_logDirectory;
    private MailboxManager m_mailboxManager;

    private List<BackupBean> m_selectedBackups;

    @Override
    public void afterResponseSent() {
        perform(m_selectedBackups);

    }

    public final void perform(List<BackupBean> backups) {
        prepare(backups);
        execute(backups, false);
    }

    public final void validate(List<BackupBean> backups) {
        prepare(backups);
        execute(backups, true);
    }

    /**
     * Do something to prepare for restoring the backup - download, unpack etc.
     *
     * @param backups list of selected BackupBean files
     */
    protected void prepare(List<BackupBean> backups) {
    }

    protected void execute(List<BackupBean> backups, boolean verify) {
        BackupBean config = null;
        BackupBean voicemail = null;
        BackupBean cdr = null;
        for (BackupBean bean : backups) {
            if (bean.getType().equals(BackupBean.Type.CONFIGURATION)) {
                config = bean;
            } else if (bean.getType().equals(BackupBean.Type.VOICEMAIL)) {
                voicemail = bean;
            } else if (bean.getType().equals(BackupBean.Type.CDR)) {
                cdr = bean;
            }
        }

        if (voicemail != null) {
            m_mailboxManager.performRestore(voicemail, verify, config != null);
        }

        if (config != null) {
            runRestoreScript(getBinDirectory(), config, verify, false);
        }

        if (cdr != null) {
            runRestoreScript(getBinDirectory(), cdr, verify, false);
        }

    }

    public static void runRestoreScript(String binDir, BackupBean backup, boolean verify, boolean noRestart) {
        String[] cmdLine = getCmdLine(binDir, backup, verify, noRestart);
        try {
            Process process = Runtime.getRuntime().exec(getCmdLine(binDir, backup, verify, noRestart));
            int code = process.waitFor();
            if (code == INCOMPATIBLE_VERSIONS && verify) {
                throw new UserException("&message.wrongVersion");
            } else if (code == INVALID_CONFIGURATION_ARCHIVE && verify) {
                throw new UserException("&message.wrongConfigurationFileToRestore");
            } else if (code == INVALID_VOICEMAIL_ARCHIVE && verify) {
                throw new UserException("&message.wrongVoicemailFileToRestore");
            }
        } catch (IOException e) {
            LOG.error(String.format(ERROR, StringUtils.join(cmdLine, SPACE)));
            throw new UserException("&message.noScriptFound");
        } catch (InterruptedException e) {
            LOG.warn(String.format(ERROR, StringUtils.join(cmdLine, SPACE)));
        }
    }

    static String[] getCmdLine(String binDir, BackupBean backup, boolean verify, boolean noRestart) {
        File executable = new File(binDir, RESTORE_BINARY);
        List<String> cmds = new ArrayList<String>();
        cmds.add(executable.getAbsolutePath());
        cmds.add(backup.getType().getOption());
        cmds.add(backup.getPath());
        cmds.add("--non-interactive");
        cmds.add("--enforce-version");
        if (verify) {
            cmds.add("--verify");
        }
        if (noRestart) {
            cmds.add("--no-restart");
        }
        return cmds.toArray(new String[cmds.size()]);
    }

    public String getBinDirectory() {
        return m_binDirectory;
    }

    public void setBinDirectory(String binDirectory) {
        m_binDirectory = binDirectory;
    }

    public String getLogDirectory() {
        return m_logDirectory;
    }

    public void setLogDirectory(String logDirectory) {
        m_logDirectory = logDirectory;
    }

    public void setMailboxManager(MailboxManager mailboxManager) {
        m_mailboxManager = mailboxManager;
    }

    public String getRestoreLogContent() {
        StringBuilder builder = new StringBuilder();
        try {
            File log = new File(getLogDirectory(), RESTORE_LOG);
            builder.append(IOUtils.toString(new FileReader(log)));
            builder.append(m_mailboxManager.getMailboxRestoreLog());
            return builder.toString();
        } catch (FileNotFoundException ex) {
            throw new UserException("&log.found.ex");
        } catch (IOException ex) {
            throw new UserException("&log.read.ex");
        }
    }

    public void setSelectedBackups(List<BackupBean> selectedBackups) {
        m_selectedBackups = selectedBackups;
    }
}
