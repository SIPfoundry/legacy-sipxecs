/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.vm;

import java.io.File;
import java.io.FileOutputStream;
import java.io.FilenameFilter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.apache.commons.httpclient.HttpClient;
import org.apache.commons.httpclient.HttpException;
import org.apache.commons.httpclient.methods.PutMethod;
import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.io.filefilter.SuffixFileFilter;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.backup.BackupBean;
import org.sipfoundry.sipxconfig.backup.Restore;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.vm.attendant.PersonalAttendant;

public class LocalMailboxManagerImpl extends AbstractMailboxManager implements MailboxManager {
    private static final String MESSAGE_SUFFIX = "-00.xml";
    private static final FilenameFilter MESSAGE_FILES = new SuffixFileFilter(MESSAGE_SUFFIX);
    private static final Log LOG = LogFactory.getLog(LocalMailboxManagerImpl.class);

    @Override
    public boolean isEnabled() {
        return getMailstoreFileDirectory() != null && getMailstoreFileDirectory().exists();
    }

    @Override
    public List<Voicemail> getVoicemail(String userId, String folder) {
        checkMailstoreDirectory();
        LocalMailbox mbox = getMailbox(userId);
        File vmdir = new File(mbox.getUserDirectory(), folder);
        String[] wavs = vmdir.list(MESSAGE_FILES);
        if (wavs == null) {
            return Collections.emptyList();
        }
        Arrays.sort(wavs);
        List<Voicemail> vms = new ArrayList<Voicemail>(wavs.length);
        for (String wav : wavs) {
            String basename = basename(wav);
            vms.add(new LocalVoicemail(getMailstoreFileDirectory(), mbox.getUserId(), folder, basename));
        }
        return vms;
    }

    @Override
    public Voicemail getVoicemail(String userId, String folder, String messageId) {
        return new LocalVoicemail(getMailstoreFileDirectory(), userId, folder, messageId);
    }

    @Override
    public void deleteMailbox(String userId) {
        deleteMailbox(getMailbox(userId));
    }

    @Override
    public void renameMailbox(String oldUserId, String newUserId) {
        LocalMailbox oldMailbox = getMailbox(oldUserId);
        LocalMailbox newMailbox = getMailbox(newUserId);
        File oldUserDir = oldMailbox.getUserDirectory();
        File newUserDir = newMailbox.getUserDirectory();
        deleteMailbox(newMailbox);
        if (oldUserDir.exists()) {
            try {
                FileUtils.moveDirectory(oldUserDir, newUserDir);
            } catch (IOException e) {
                throw new MailstoreMisconfigured("Cannot rename mailbox directory " + oldUserDir.getAbsolutePath(),
                        e);
            }
        }
    }

    @Override
    public void saveDistributionLists(String userId, DistributionList[] lists) {
        LocalMailbox mailbox = getMailbox(userId);
        Collection<String> aliases = DistributionList.getUniqueExtensions(lists);
        getCoreContext().checkForValidExtensions(aliases, PermissionName.VOICEMAIL);
        File file = mailbox.getDistributionListsFile();
        getDistributionListsWriter().writeObject(lists, file);
    }

    @Override
    public DistributionList[] loadDistributionLists(String userId) {
        LocalMailbox mailbox = getMailbox(userId);
        File file = mailbox.getDistributionListsFile();
        DistributionList[] lists = getDistributionListsReader().readObject(file);
        if (lists == null) {
            lists = DistributionList.createBlankList();
        }
        return lists;
    }

    @Override
    public void markRead(String userId, String messageId) {
        LocalMailbox mailbox = getMailbox(userId);
        StringBuilder sb = new StringBuilder(PATH_MAILBOX).append(mailbox.getUserDirectory().getName())
                .append(PATH_MESSAGE).append(messageId).append("/heard");
        invokeWebservice(sb.toString(), mailbox.getUserId());
    }

    @Override
    public void move(String userId, Voicemail voicemail, String destinationFolderId) {
        LocalMailbox mailbox = getMailbox(userId);
        File destination = new File(mailbox.getUserDirectory(), destinationFolderId);
        // make sure destination directory exists
        if (!destination.isDirectory()) {
            destination.mkdir();
        }
        for (File f : ((LocalVoicemail) voicemail).getAllFiles()) {
            f.renameTo(new File(destination, f.getName()));
        }

        // ask voicemail to update the MWI ..
        StringBuilder sb = new StringBuilder(PATH_MAILBOX).append(mailbox.getUserDirectory().getName()).append(
                "/mwi");
        invokeWebservice(sb.toString(), mailbox.getUserId());

    }

    @Override
    public void delete(String userId, Voicemail voicemail) {
        LocalMailbox mailbox = getMailbox(userId);
        StringBuilder sb = new StringBuilder(PATH_MAILBOX).append(mailbox.getUserDirectory().getName())
                .append(PATH_MESSAGE).append(voicemail.getMessageId()).append("/delete");
        invokeWebservice(sb.toString(), mailbox.getUserId());
    }

    @Override
    public void save(Voicemail voicemail) {
        LocalVoicemail vm = (LocalVoicemail) voicemail;
        FileOutputStream out = null;
        try {
            out = new FileOutputStream(vm.getDescriptorFile());
            vm.writeMessageDescriptor(vm.getDescriptor(), out);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            IOUtils.closeQuietly(out);
        }
    }

    @Override
    public boolean performBackup(File workingDir) {
        try {
            ProcessBuilder pb = new ProcessBuilder(getBinDir() + File.separator + "sipx-backup", "-n", "-v");
            Process process = pb.directory(workingDir).start();
            int code = process.waitFor();
            if (code != 0) {
                String errorMsg = String.format("Voicemail backup operation failed. Exit code: %d", code);
                LOG.error(errorMsg);
                return false;
            }
        } catch (Exception e) {
            LOG.error(String.format("Voicemail backup operation failed, exception %s", e.getMessage()));
            return false;
        }
        return true;
    }

    @Override
    public void performRestore(BackupBean archivePath, boolean verify, boolean noRestart) {
        Restore.runRestoreScript(getBinDir(), archivePath, verify, noRestart);
    }

    private void invokeWebservice(String uri, String username) {
        PutMethod httpPut = null;
        try {
            HttpClient client = new HttpClient();
            httpPut = new PutMethod(getMailboxServerUrl() + uri);
            int statusCode = client.executeMethod(httpPut);
            if (statusCode != 200) {
                throw new UserException("&error.https.server.status.code", getHost(), String.valueOf(statusCode));
            }
        } catch (HttpException ex) {
            throw new UserException("&error.https.server", getHost(), ex.getMessage());
        } catch (IOException ex) {
            throw new UserException("&error.io.exception", getHost(), ex.getMessage());
        } finally {
            if (httpPut != null) {
                httpPut.releaseConnection();
            }
        }
    }

    @Override
    public String getMailboxRestoreLog() {
        return StringUtils.EMPTY;
    }

    /**
     * Because in HA systems, admin may change mailstore directory, validate it
     */
    void checkMailstoreDirectory() {
        if (getMailstoreFileDirectory() == null) {
            throw new MailstoreMisconfigured(null);
        }
        if (!getMailstoreFileDirectory().exists()) {
            throw new MailstoreMisconfigured(getMailstoreFileDirectory().getAbsolutePath());
        }
    }

    static class MailstoreMisconfigured extends UserException {
        MailstoreMisconfigured() {
            super("Mailstore directory configuration setting is missing.");
        }

        MailstoreMisconfigured(String dir) {
            super(String.format("Mailstore directory does not exist '%s'", dir));
        }

        MailstoreMisconfigured(String message, IOException cause) {
            super(message, cause);
        }
    }

    /**
     * extract file name w/o ext.
     */
    static String basename(String filename) {
        int suffix = filename.lastIndexOf(MESSAGE_SUFFIX);
        return suffix >= 0 ? filename.substring(0, suffix) : filename;
    }

    public LocalMailbox getMailbox(String userId) {
        return new LocalMailbox(getMailstoreFileDirectory(), userId);
    }

    private void deleteMailbox(LocalMailbox mailbox) {
        File userDir = mailbox.getUserDirectory();
        if (userDir.exists()) {
            try {
                FileUtils.deleteDirectory(userDir);
            } catch (IOException e) {
                throw new MailstoreMisconfigured("Cannot delete mailbox directory " + userDir.getAbsolutePath(), e);
            }
        }
    }

    public void writePersonalAttendant(PersonalAttendant pa) {
        LocalMailbox mailbox = getMailbox(pa.getUser().getUserName());
        getPersonalAttendantWriter().write((LocalMailbox) mailbox, pa);
    }

    public void writePreferencesFile(User user) {
        LocalMailbox mailbox = getMailbox(user.getUserName());
        File file = mailbox.getVoicemailPreferencesFile();
        getMailboxPreferencesWriter().writeObject(new MailboxPreferences(user), file);
    }

}
