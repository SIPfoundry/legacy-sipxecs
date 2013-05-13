/**
 *
 *
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.voicemail.mailbox;

import java.io.File;
import java.io.FilenameFilter;
import java.io.IOException;
import java.util.Arrays;
import java.util.Comparator;
import java.util.LinkedList;
import java.util.List;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.filefilter.RegexFileFilter;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.voicemail.mailbox.MessageDescriptor.Priority;

public class FilesystemMailboxManager extends AbstractMailboxManager {
    private static final Comparator<File> FILE_DATE_COMPARATOR = new FileDateComparator();
    private static final String AUDIO_IDENTIFIER = "-00.%s";
    private static final String MESSAGE_IDENTIFIER = "-00.xml";
    private static final String ORIGINAL_MESSAGE_IDENTIFIER = "-01.xml";
    private static final String STATUS_IDENTIFIER = "-00.sta";
    private static final String URGENT_IDENTIFIER = "-00.urg";
    private static final String MESSAGE_REGEX = "^(\\d+)-00\\.xml$";
    private static final String STATUS_REGEX = "^(\\d+)-00\\.sta$";
    private static final String MESSAGEID_FILE_REGEX = "^%s-00.*";
    private static final String ORIGINAL_AUDIO_IDENTIFIER = "-01.%s";
    private static final String FW_AUDIO_IDENTIFIER = "-FW.%s";
    private MessageDescriptorWriter m_descriptorWriter;
    private MessageDescriptorReader m_descriptorReader;

    public void init() {
        File mailstore = new File(m_mailstoreDirectory);
        if (!mailstore.exists()) {
            mailstore.mkdir();
        }
    }

    @Override
    public MailboxDetails getMailboxDetails(String username) {
        FilenameFilter filter = new RegexFileFilter(MESSAGE_REGEX);
        List<String> inboxMessages = extractMessages(getFolder(username, Folder.INBOX).listFiles(filter));
        List<String> savedMessages = extractMessages(getFolder(username, Folder.SAVED).listFiles(filter));
        List<String> deletedMessages = extractMessages(getFolder(username, Folder.DELETED).listFiles(filter));
        List<String> conferenceMessages = extractMessages(getFolder(username, Folder.CONFERENCE).listFiles(filter));

        FilenameFilter unheardFilter = new RegexFileFilter(STATUS_REGEX);
        List<String> unheardMessages = extractMessages(getFolder(username, Folder.INBOX).listFiles(unheardFilter));
        return new MailboxDetails(username, inboxMessages, savedMessages, deletedMessages, conferenceMessages,
                unheardMessages);
    }

    @Override
    protected VmMessage saveTempMessageInStorage(User destUser, TempMessage message, MessageDescriptor descriptor,
            Folder storageFolder, String messageId) {
        File folderDir = getFolder(destUser.getUserName(), storageFolder);
        File audioFile = new File(folderDir, messageId + String.format(AUDIO_IDENTIFIER, getAudioFormat()));
        File descriptorFile = new File(folderDir, messageId + MESSAGE_IDENTIFIER);
        File statusFile = new File(folderDir, messageId + STATUS_IDENTIFIER);
        File urgentFile = new File(folderDir, messageId + URGENT_IDENTIFIER);
        String operation = "storing stuff";
        boolean urgent = message.getPriority() == Priority.URGENT;
        try {
            operation = "creating status file " + statusFile.getPath();
            LOG.debug("FileSystemMailboxManager::newMessage " + operation);
            FileUtils.touch(statusFile);

            if (urgent) {
                operation = "creating urgent file " + urgentFile.getPath();
                LOG.debug("FileSystemMailboxManager::newMessage " + operation);
                FileUtils.touch(urgentFile);
            }

            operation = "copying recording file to " + audioFile.getPath();
            LOG.debug("VmMessage::newMessage " + operation);
            FileUtils.copyFile(new File(message.getTempPath()), audioFile, true);

            operation = "creating messageDescriptor " + descriptorFile.getPath();
            LOG.debug("VmMessage::newMessage " + operation);
            m_descriptorWriter.writeObject(descriptor, descriptorFile);
        } catch (IOException e) {
            LOG.error("VmMessage::newMessage error while " + operation, e);
            return null;
        }
        if (storageFolder == Folder.INBOX) {
            m_mwi.sendMWI(destUser, getMailboxDetails(destUser.getUserName()));
        }
        LOG.info("VmMessage::newMessage created message " + descriptorFile.getPath());
        return new VmMessage(messageId, audioFile, descriptor, urgent);
    }

    //Automatically saves in INBOX
    @Override
    protected VmMessage saveTempMessageInStorage(User destUser, TempMessage message, MessageDescriptor descriptor,
            String messageId) {
        return saveTempMessageInStorage(destUser, message, descriptor, Folder.INBOX, messageId);
    }

    @Override
    protected VmMessage copyMessage(String newMessageId, User destUser, TempMessage message) {
        // destination files
        File destinationInbox = getFolder(destUser.getUserName(), Folder.INBOX);
        File destStatus = new File(destinationInbox, newMessageId + STATUS_IDENTIFIER);
        File destUrg = new File(destinationInbox, newMessageId + URGENT_IDENTIFIER);
        File destAudio = new File(destinationInbox, newMessageId + String.format(AUDIO_IDENTIFIER, getAudioFormat()));
        File destDescriptor = new File(destinationInbox, newMessageId + MESSAGE_IDENTIFIER);
        File originalDestAudio = new File(destinationInbox, newMessageId
                + String.format(ORIGINAL_AUDIO_IDENTIFIER, getAudioFormat()));
        File destCombined = new File(destinationInbox, newMessageId
                + String.format(FW_AUDIO_IDENTIFIER, getAudioFormat()));

        // original files
        File originalInbox = getFolder(message.getCurrentUser(), Folder.INBOX);
        FilenameFilter filter = new RegexFileFilter(String.format(MESSAGEID_FILE_REGEX, message.getSavedMessageId()));
        File[] originalFiles = originalInbox.listFiles(filter);
        boolean urgent = false;
        MessageDescriptor descriptor = null;

        String operation = "copying stuff";
        try {
            operation = "creating status file " + destStatus.getPath();
            FileUtils.touch(destStatus);
            for (File originalFile : originalFiles) {
                if (originalFile.getName().endsWith(URGENT_IDENTIFIER)) {
                    operation = "creating urgent file " + destUrg.getPath();
                    FileUtils.touch(destUrg);
                    urgent = true;
                } else if (originalFile.getName().endsWith(String.format(AUDIO_IDENTIFIER, getAudioFormat()))) {
                    operation = "copying audio file " + destAudio.getPath();
                    FileUtils.copyFile(originalFile, destAudio);
                } else if (originalFile.getName().endsWith(MESSAGE_IDENTIFIER)) {
                    operation = "copying descriptor file " + destDescriptor.getPath();
                    descriptor = m_descriptorReader.readObject(originalFile);
                    if (descriptor.getOtherRecipients() != null) {
                        descriptor.addOtherRecipient(ValidUsers.getUserPart(descriptor.getId()));
                        descriptor.removeOtherRecipient(destUser.getUserName());
                    }
                    descriptor.setId(destUser.getIdentity());
                    descriptor.setSubject("Voice Message " + newMessageId);
                    m_descriptorWriter.writeObject(descriptor, destDescriptor);
                } else if (originalFile.getName().endsWith(
                        String.format(ORIGINAL_AUDIO_IDENTIFIER, getAudioFormat()))) {
                    operation = "copying original audio file " + originalDestAudio.getPath();
                    FileUtils.copyFile(originalFile, originalDestAudio);
                } else if (originalFile.getName().endsWith(String.format(FW_AUDIO_IDENTIFIER, getAudioFormat()))) {
                    operation = "copying combined audio file " + destCombined.getPath();
                    FileUtils.copyFile(originalFile, destCombined);
                }
            }
        } catch (IOException e) {
            LOG.error("VmMessage::copy error while " + operation, e);
            return null;
        }
        m_mwi.sendMWI(destUser, getMailboxDetails(destUser.getUserName()));
        if (destCombined.exists()) {
            return new VmMessage(newMessageId, destCombined, descriptor, urgent);
        } else {
            return new VmMessage(newMessageId, destAudio, descriptor, urgent);
        }

    }

    @Override
    public VmMessage getVmMessage(String username, Folder folder, String messageId, boolean loadAudio) {
        File mailboxFolder = getFolder(username, folder);
        FilenameFilter filter = new RegexFileFilter(String.format("^%s%s", messageId, MESSAGE_IDENTIFIER));
        File[] files = mailboxFolder.listFiles(filter);
        if (files != null && files.length != 1) {
            throw new MessageNotFoundException();
        }
        MessageDescriptor descriptor = m_descriptorReader.readObject(files[0]);
        FilenameFilter filterById = new FileFilterByMessageId(messageId);
        File[] audioFiles = mailboxFolder.listFiles(filterById);
        File audioFile = null;
        File originalFile = null;
        File combinedFile = null;
        boolean unheard = false;
        boolean urgent = false;
        for (File file : audioFiles) {
            String fileName = file.getName();
            if (fileName.endsWith(String.format(AUDIO_IDENTIFIER, getAudioFormat()))
                    || fileName.endsWith(String.format(AUDIO_IDENTIFIER, getAltAudioFormat()))) {
                audioFile = file;
            } else if (fileName.endsWith(String.format(ORIGINAL_AUDIO_IDENTIFIER, getAudioFormat()))
                    || fileName.endsWith(String.format(ORIGINAL_AUDIO_IDENTIFIER, getAltAudioFormat()))) {
                originalFile = file;
            } else if (fileName.endsWith(String.format(FW_AUDIO_IDENTIFIER, getAudioFormat()))
                    || fileName.endsWith(String.format(FW_AUDIO_IDENTIFIER, getAltAudioFormat()))) {
                combinedFile = file;
            } else if (fileName.endsWith(STATUS_IDENTIFIER)) {
                unheard = true;
            } else if (fileName.endsWith(URGENT_IDENTIFIER)) {
                urgent = true;
            }
        }
        if (combinedFile != null) {
            return new VmMessage(messageId, username, combinedFile, descriptor, folder, unheard, urgent);
        }
        if (audioFile != null) {
            return new VmMessage(messageId, username, audioFile, descriptor, folder, unheard, urgent);
        }
        return new VmMessage(messageId, username, originalFile, descriptor, folder, unheard, urgent);
    }

    @Override
    public void saveMessage(User user, VmMessage message) {
        Folder messageFolder = message.getParentFolder();
        if (messageFolder == Folder.SAVED) {
            return;
        }
        try {
            boolean sendMwi = false;
            FilenameFilter filterById = new FileFilterByMessageId(message.getMessageId());
            File[] messageFiles = getFolder(message.getUserName(), messageFolder).listFiles(filterById);
            if (messageFolder == Folder.INBOX) {
                File savedFolder = getFolder(message.getUserName(), Folder.SAVED);
                for (File file : messageFiles) {
                    String fileName = file.getName();
                    // mark heard
                    if (fileName.endsWith(URGENT_IDENTIFIER)) {
                        FileUtils.deleteQuietly(file);
                    } else {
                        FileUtils.moveFileToDirectory(file, savedFolder, true);
                    }
                }
                sendMwi = true;
            } else if (messageFolder == Folder.DELETED) {
                File inboxFolder = getFolder(message.getUserName(), Folder.INBOX);
                for (File file : messageFiles) {
                    FileUtils.moveFileToDirectory(file, inboxFolder, true);
                }
                sendMwi = true;
            }
            if (sendMwi) {
                m_mwi.sendMWI(user, getMailboxDetails(user.getUserName()));
            }
        } catch (IOException ex) {
            LOG.error("Failed to save message", ex);
        }
    }

    @Override
    public void deleteMessage(User user, VmMessage message) {
        try {
            Folder messageFolder = message.getParentFolder();
            FilenameFilter filterById = new FileFilterByMessageId(message.getMessageId());
            File[] messageFiles = getFolder(message.getUserName(), messageFolder).listFiles(filterById);
            if (messageFolder == Folder.DELETED) {
                for (File messageFile : messageFiles) {
                    FileUtils.deleteQuietly(messageFile);
                }
            } else if (messageFolder == Folder.INBOX || messageFolder == Folder.SAVED) {
                File deletedFolder = getFolder(message.getUserName(), Folder.DELETED);
                for (File file : messageFiles) {
                    String fileName = file.getName();
                    // mark heard
                    if (fileName.endsWith(URGENT_IDENTIFIER)) {
                        FileUtils.deleteQuietly(file);
                    } else {
                        FileUtils.moveFileToDirectory(file, deletedFolder, true);
                    }
                }
                if (messageFolder == Folder.INBOX) {
                    m_mwi.sendMWI(user, getMailboxDetails(user.getUserName()));
                }
            }
        } catch (IOException ex) {
            LOG.error("Failed to save message", ex);
        }
    }

    @Override
    public void markMessageHeard(User user, VmMessage message) {
        FilenameFilter fileStatusFilter = new RegexFileFilter(String.format("^%s%s", message.getMessageId(),
                STATUS_IDENTIFIER));
        File[] status = getFolder(message.getUserName(), message.getParentFolder()).listFiles(fileStatusFilter);
        if (status != null && status.length == 1) {
            FileUtils.deleteQuietly(status[0]);
            m_mwi.sendMWI(user, getMailboxDetails(user.getUserName()));
        }
    }

    @Override
    public void markMessageHeard(User user, String messageId) {
        FilenameFilter filter = new FileFilterByMessageId(messageId);
        File[] files = findFilesInUserDirectory(user.getUserName(), filter);
        File statusFile = null;
        for (File file : files) {
            if (file.getName().endsWith(STATUS_IDENTIFIER)) {
                statusFile = file;
                break;
            }
        }
        if (statusFile != null) {
            FileUtils.deleteQuietly(statusFile);
            m_mwi.sendMWI(user, getMailboxDetails(user.getUserName()));
        }
    }

    @Override
    public void removeDeletedMessages(String username) {
        try {
            FileUtils.deleteDirectory(getFolder(username, Folder.DELETED));
        } catch (IOException ex) {
            LOG.error("cannot delete deleted directory for user " + username);
        }
    }

    @Override
    protected File getTempFolder(String username) {
        return getFolder(username, Folder.DELETED);
    }

    @Override
    protected VmMessage forwardMessage(VmMessage originalMessage, TempMessage comments,
            MessageDescriptor descriptor, User destUser, String newMessageId) {
        boolean urgent = false;
        File destCombined = null;
        try {
            // destination files
            File destinationInbox = getFolder(destUser.getUserName(), Folder.INBOX);
            File destStatus = new File(destinationInbox, newMessageId + STATUS_IDENTIFIER);
            File destAudio = new File(destinationInbox, newMessageId
                    + String.format(AUDIO_IDENTIFIER, getAudioFormat()));
            File destUrg = new File(destinationInbox, newMessageId + URGENT_IDENTIFIER);
            File originalDestAudio = new File(destinationInbox, newMessageId
                    + String.format(ORIGINAL_AUDIO_IDENTIFIER, getAudioFormat()));
            destCombined = new File(destinationInbox, newMessageId
                    + String.format(FW_AUDIO_IDENTIFIER, getAudioFormat()));
            File originalDestDescriptor = new File(destinationInbox, newMessageId + ORIGINAL_MESSAGE_IDENTIFIER);
            File destDescriptor = new File(destinationInbox, newMessageId + MESSAGE_IDENTIFIER);
            FileUtils.touch(destStatus);
            if (comments.getTempPath() != null) {
                FileUtils.copyFile(new File(comments.getTempPath()), destAudio, true);
            } else {
                FileUtils.touch(destAudio);
            }

            // original files
            Folder originalFolder = originalMessage.getParentFolder();
            FilenameFilter filterById = new FileFilterByMessageId(originalMessage.getMessageId());
            File[] filesToForward = getFolder(originalMessage.getUserName(), originalFolder).listFiles(filterById);

            for (File fileToForward : filesToForward) {
                if (fileToForward.getName().endsWith(URGENT_IDENTIFIER)) {
                    FileUtils.touch(destUrg);
                    urgent = true;
                } else if (fileToForward.getName().endsWith(String.format(AUDIO_IDENTIFIER, getAudioFormat()))) {
                    FileUtils.copyFile(fileToForward, originalDestAudio, true);
                    if (comments.getTempPath() != null) {
                        concatAudio(destCombined, destAudio, fileToForward);
                    } else {
                        FileUtils.copyFile(fileToForward, destCombined, true);
                    }
                } else if (fileToForward.getName().endsWith(MESSAGE_IDENTIFIER)) {
                    FileUtils.copyFile(fileToForward, originalDestDescriptor, true);
                }
            }

            m_descriptorWriter.writeObject(descriptor, destDescriptor);
            m_mwi.sendMWI(destUser, getMailboxDetails(destUser.getUserName()));
        } catch (Exception ex) {
            LOG.error("Failed to forward message", ex);
            return null;
        }
        return new VmMessage(newMessageId, destCombined, descriptor, urgent);
    }

    @Override
    public File getRecordedName(String username) {
        return new File(getUserDirectory(username), getNameFile());
    }

    @Override
    public void saveRecordedName(TempMessage message) {
        try {
            FileUtils.copyFile(new File(message.getTempPath()), getRecordedName(message.getCurrentUser()));
        } catch (IOException ex) {
            LOG.error("Failed to save recorded name", ex);
        }
    }

    @Override
    public void saveCustomAutoattendantPrompt(TempMessage message) {
        try {
            String aaName = getPromptFile();
            File aaFile = new File(m_promptsDirectory, aaName);
            FileUtils.copyFile(new File(message.getTempPath()), aaFile);
        } catch (IOException ex) {
            LOG.error("Failed to save recorded name", ex);
        }
    }

    @Override
    public void saveGreetingFile(GreetingType type, TempMessage recording) {
        try {
            File greetingFile = new File(getUserDirectory(recording.getCurrentUser()), getGreetingTypeName(type));
            FileUtils.copyFile(new File(recording.getTempPath()), greetingFile);
            // ExtMailStore.SaveGreetingInFolder(m_controller.getMailbox(), type, greetingFile);
        } catch (IOException ex) {
            LOG.error("Failed to save recorded name", ex);
        }
    }

    @Override
    public String getGreetingPath(User user, GreetingType type) {
        String greetingTypeName = getGreetingTypeName(type);
        if (StringUtils.isNotEmpty(greetingTypeName)) {
            File greeting = new File(getUserDirectory(user.getUserName()), greetingTypeName);
            if (greeting.exists()) {
                return greeting.getPath();
            }
        }
        return null;
    }

    @Override
    public List<VmMessage> getMessages(String username, Folder folder) {
        File mailboxFolder = getFolder(username, folder);
        List<VmMessage> messages = new LinkedList<VmMessage>();
        File[] files = mailboxFolder.listFiles(new MessageCountFilter());
        Arrays.sort(files, FILE_DATE_COMPARATOR);
        for (File file : files) {
            MessageDescriptor descriptor = m_descriptorReader.readObject(file);
            String messageId = StringUtils.removeEnd(file.getName(), MESSAGE_IDENTIFIER);
            boolean unheard = new File(mailboxFolder, String.format("%s%s", messageId, STATUS_IDENTIFIER)).exists();
            boolean urgent = new File(mailboxFolder, String.format("%s%s", messageId, URGENT_IDENTIFIER)).exists();
            messages.add(new VmMessage(messageId, username, null, descriptor, folder, unheard, urgent));
        }
        return messages;
    }

    @Override
    public VmMessage getVmMessage(String username, String messageId, boolean loadAudio) {
        FilenameFilter filter = new FileFilterByMessageId(messageId);
        File[] files = findFilesInUserDirectory(username, filter);
        MessageDescriptor descriptor = null;
        File audioFile = null;
        File originalFile = null;
        File combinedFile = null;
        boolean unheard = false;
        boolean urgent = false;
        for (File file : files) {
            if (file.getName().endsWith(String.format(AUDIO_IDENTIFIER, getAudioFormat()))
                    || file.getName().endsWith(String.format(AUDIO_IDENTIFIER, getAltAudioFormat()))) {
                audioFile = file;
            } else if (file.getName().endsWith(String.format(ORIGINAL_AUDIO_IDENTIFIER, getAudioFormat()))
                    || file.getName().endsWith(String.format(ORIGINAL_AUDIO_IDENTIFIER, getAltAudioFormat()))) {
                originalFile = file;
            } else if (file.getName().endsWith(String.format(FW_AUDIO_IDENTIFIER, getAudioFormat()))
                    || file.getName().endsWith(String.format(FW_AUDIO_IDENTIFIER, getAltAudioFormat()))) {
                combinedFile = file;
            } else if (file.getName().endsWith(MESSAGE_IDENTIFIER)) {
                descriptor = m_descriptorReader.readObject(file);
                descriptor.setFilePath(file.getPath());
            } else if (file.getName().endsWith(STATUS_IDENTIFIER)) {
                unheard = true;
            } else if (file.getName().endsWith(URGENT_IDENTIFIER)) {
                urgent = true;
            }
        }
        if (combinedFile != null) {
            return new VmMessage(messageId, username, combinedFile, descriptor, null, unheard, urgent);
        }
        if (audioFile != null) {
            return new VmMessage(messageId, username, audioFile, descriptor, null, unheard, urgent);
        }
        return new VmMessage(messageId, username, originalFile, descriptor, null, unheard, urgent);
    }

    private File[] findFilesInUserDirectory(String username, FilenameFilter filter) {
        File[] files = findFilesInFolder(getFolder(username, Folder.INBOX), filter);
        if (files != null) {
            return files;
        }
        files = findFilesInFolder(getFolder(username, Folder.SAVED), filter);
        if (files != null) {
            return files;
        }
        files = findFilesInFolder(getFolder(username, Folder.DELETED), filter);
        if (files != null) {
            return files;
        }
        files = findFilesInFolder(getFolder(username, Folder.CONFERENCE), filter);
        if (files != null) {
            return files;
        }
        throw new MessageNotFoundException();
    }

    private File[] findFilesInInbox(String username, FilenameFilter filter) {
        File[] files = findFilesInFolder(getFolder(username, Folder.INBOX), filter);
        if (files != null) {
            return files;
        }
        throw new MessageNotFoundException();
    }

    private File[] findFilesInFolder(File folder, FilenameFilter filter) {
        File[] files = folder.listFiles(filter);
        if (files != null && files.length > 0) {
            return files;
        }
        return null;
    }

    @Override
    public void markMessageUnheard(User user, String messageId) {
        FilenameFilter filter = new FileFilterByMessageId(messageId);
        File[] files = findFilesInInbox(user.getUserName(), filter);
        try {
            for (File file : files) {
                File statusFile = new File(file.getParentFile(), messageId + STATUS_IDENTIFIER);
                FileUtils.touch(statusFile);
                m_mwi.sendMWI(user, getMailboxDetails(user.getUserName()));
                break;
            }
        } catch (IOException ex) {
            LOG.error(String.format("failed to mark message %s unheard for user %s", messageId, user.getUserName()),
                    ex);
        }
    }

    @Override
    public boolean isMessageUnHeard(User user, String messageId) {
        VmMessage vmMessage = getVmMessage(user.getUserName(), Folder.INBOX, messageId, false);
        return vmMessage.isUnHeard();
    }

    @Override
    public void deleteMessage(User user, String messageId) {
        FilenameFilter filter = new FileFilterByMessageId(messageId);
        File[] files = findFilesInUserDirectory(user.getUserName(), filter);
        for (File file : files) {
            FileUtils.deleteQuietly(file);
        }
    }

    @Override
    public void updateMessageSubject(User user, String messageId, String subject) {
        VmMessage message = getVmMessage(user.getUserName(), messageId, false);
        MessageDescriptor descriptor = message.getDescriptor();
        descriptor.setSubject(subject);
        m_descriptorWriter.writeObject(descriptor, new File(descriptor.getFilePath()));
    }

    @Override
    public void moveMessageToFolder(User user, String messageId, String destination) {
        FilenameFilter filter = new FileFilterByMessageId(messageId);
        File destinationFile = getFolder(user.getUserName(), getFolderFromName(destination));
        File[] files = findFilesInUserDirectory(user.getUserName(), filter);
        try {
            for (File file : files) {
                FileUtils.moveFileToDirectory(file, destinationFile, true);
            }
        } catch (IOException ex) {
            LOG.error(
                    String.format("failed to move message %s in %s for user %s", messageId, destination,
                            user.getUserName()), ex);
        }

    }

    @Override
    public void deleteMailbox(String username) {
        try {
            File mailbox = getUserDirectory(username);
            FileUtils.deleteDirectory(mailbox);
        } catch (IOException ex) {
            LOG.error(String.format("failed to delete mailbox for user %s", username), ex);
        }
    }

    @Override
    public void renameMailbox(User user, String oldUser) {
        try {
            File oldUserDir = getUserDirectory(oldUser);
            File newUserDir = getUserDirectory(user.getUserName());
            FileUtils.copyDirectory(oldUserDir, newUserDir);
            FileUtils.deleteDirectory(oldUserDir);
        } catch (IOException ex) {
            LOG.error(String.format("failed to delete mailbox for user %s", user.getUserName()), ex);
        }
    }

    private File getFolder(String username, Folder folder) {
        File file = new File(getUserDirectory(username), folder.toString());
        if (!file.exists()) {
            file.mkdirs();
        }
        return file;
    }

    private File getUserDirectory(String username) {
        return new File(m_mailstoreDirectory + File.separator + username);
    }

    private List<String> extractMessages(File[] files) {
        Arrays.sort(files, new Comparator<File>() {
            @Override
            public int compare(File f1, File f2) {
                return Long.valueOf(f1.lastModified()).compareTo(f2.lastModified());
            }
        });
        List<String> messageList = new LinkedList<String>();
        for (File file : files) {
            messageList.add(StringUtils.removeEnd(file.getName(), MESSAGE_IDENTIFIER));
        }
        return messageList;
    }

    private static class FileFilterByMessageId implements FilenameFilter {
        private final String m_messageIdPrefix;

        FileFilterByMessageId(String messageId) {
            m_messageIdPrefix = messageId + "-";
        }

        @Override
        public boolean accept(File dir, String name) {
            return name.startsWith(m_messageIdPrefix);
        }
    }

    private static class MessageCountFilter implements FilenameFilter {
        @Override
        public boolean accept(File dir, String name) {
            return name.endsWith(MESSAGE_IDENTIFIER);
        }
    }

    private static class FileDateComparator implements Comparator<File> {
        @Override
        public int compare(File file1, File file2) {
            long result = file1.lastModified() - file2.lastModified();
            if (result < 0) {
                return -1;
            } else if (result > 0) {
                return 1;
            } else {
                return 0;
            }
        }
    }

    public void setMessageDescriptorWriter(MessageDescriptorWriter writer) {
        m_descriptorWriter = writer;
    }

    public void setMessageDescriptorReader(MessageDescriptorReader reader) {
        m_descriptorReader = reader;
    }

}
