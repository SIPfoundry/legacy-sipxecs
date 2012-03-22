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
import java.io.FileWriter;
import java.io.IOException;
import java.io.Serializable;
import java.io.Writer;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigCommands;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.WaitingListener;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.springframework.beans.factory.annotation.Required;

/**
 * Interface to command line restore utility
 */
public class Restore implements Serializable, WaitingListener {
    private static final Log LOG = LogFactory.getLog(Restore.class);
    private static final String ERROR = "Errors when executing restore script: %s";
    private static final String RESTORE_LOG = "sipx-restore.log";
    private static final String RESTORE_SCRIPT = "sipx-restore";
    private static final String SEPARATOR = " ";
    private static final int INCOMPATIBLE_VERSIONS = 5;
    private static final int INVALID_CONFIGURATION_ARCHIVE = 7;
    private static final int INVALID_VOICEMAIL_ARCHIVE = 8;
    private String m_logDirectory;
    private String m_binDirectory;
    private MailboxManager m_mailboxManager;
    private List<BackupBean> m_selectedBackups;
    private ConfigCommands m_configCommands;
    private LocationsManager m_locationsManager;

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
            if (verify) {
                // verify locally if valid archive
                runRestoreScript(voicemail, verify, true);
            } else {
                m_mailboxManager.performRestore(voicemail, config != null);
            }
        }

        if (cdr != null) {
            runRestoreScript(cdr, verify, false);
        }

        if (config != null) {
            runRestoreScript(config, verify, false);
        }
    }

    public void runRestoreScript(BackupBean backup, boolean verify, boolean noRestart) {
        if (verify) {
            // run directly restore script if validating archives only, otherwise it needs to be
            // run as root, so drive this through cfengine
            String cmdLine = composeCmdLine(backup, verify, noRestart);
            try {
                File script = new File(m_binDirectory, RESTORE_SCRIPT);
                Process process = Runtime.getRuntime().exec(script.getAbsolutePath() + SEPARATOR + cmdLine);
                int code = process.waitFor();
                if (code == INCOMPATIBLE_VERSIONS && verify) {
                    throw new UserException("&message.wrongVersion");
                } else if (code == INVALID_CONFIGURATION_ARCHIVE && verify) {
                    throw new UserException("&message.wrongConfigurationFileToRestore");
                } else if (code == INVALID_VOICEMAIL_ARCHIVE && verify) {
                    throw new UserException("&message.wrongVoicemailFileToRestore");
                }
            } catch (IOException e) {
                LOG.error(String.format(ERROR, cmdLine));
                throw new UserException("&message.noScriptFound");
            } catch (InterruptedException e) {
                LOG.warn(String.format(ERROR, cmdLine));
            }
        } else {
            Writer wtr = null;
            try {
                Location primary = m_locationsManager.getPrimaryLocation();
                File f = new File(((ConfigManager) m_configCommands).getLocationDataDirectory(primary),
                        "restore.ini");
                wtr = new FileWriter(f);
                wtr.write(composeCmdLine(backup, verify, noRestart));
                wtr.flush();
                m_configCommands.runRestoreScript(primary);
            } catch (IOException ex) {
                LOG.error(String.format(ERROR, ex.getMessage()));
                throw new UserException("&err.restore.failed");
            } finally {
                IOUtils.closeQuietly(wtr);
            }
        }
    }

    String composeCmdLine(BackupBean backup, boolean verify, boolean noRestart) {
        List<String> cmds = new ArrayList<String>();
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
        return StringUtils.join(cmds, SEPARATOR);
    }

    public String getLogDirectory() {
        return m_logDirectory;
    }

    @Required
    public void setBinDirectory(String binDirectory) {
        m_binDirectory = binDirectory;
    }

    @Required
    public void setLogDirectory(String logDirectory) {
        m_logDirectory = logDirectory;
    }

    @Required
    public void setMailboxManager(MailboxManager mailboxManager) {
        m_mailboxManager = mailboxManager;
    }

    @Required
    public void setConfigCommands(ConfigCommands configCommands) {
        m_configCommands = configCommands;
    }

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
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
