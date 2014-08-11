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
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.SequenceInputStream;

import javax.sound.sampled.AudioFileFormat;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.sipxivr.email.Emailer;
import org.sipfoundry.sipxivr.rest.RestfulRequest;
import org.sipfoundry.voicemail.Mwi;
import org.sipfoundry.voicemail.mailbox.MessageDescriptor.Priority;

public abstract class AbstractMailboxManager implements MailboxManager {

    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    protected String m_mailstoreDirectory;
    protected String m_promptsDirectory;
    protected String m_operatorAddr;
    protected Mwi m_mwi;
    private String m_configUrl;
    private String m_secret;
    private Emailer m_emailer;
    private String m_identity;
    private String m_audioFormat;
    private String m_altAudioFormat;

    protected abstract VmMessage saveTempMessageInStorage(User destUser, TempMessage message,
            MessageDescriptor descriptor, Folder storageFolder, String messageId);

    protected abstract VmMessage saveTempMessageInStorage(User destUser, TempMessage message,
            MessageDescriptor descriptor, String messageId);

    protected abstract File getTempFolder(String username);

    protected abstract VmMessage copyMessage(String newMessageId, User destUser, TempMessage message);

    protected abstract VmMessage forwardMessage(VmMessage originalMessage, TempMessage comments,
            MessageDescriptor descriptor, User destUser, String newMessageId);

    @Override
    public final TempMessage createTempMessage(String username, String fromUri, boolean audio) {
        return createTempMessage(username, fromUri, m_audioFormat, audio);
    }

    @Override
    public final TempMessage createTempMessage(String username, String fromUri, String extension, boolean audio) {
        try {
            String audioPath = null;
            if (audio) {
                File audioFile = File.createTempFile("temp_recording_", "." + extension, getTempFolder(username));
                audioPath = audioFile.getPath();
            }
            return new TempMessage(username, audioPath, fromUri, Priority.NORMAL, null);
        } catch (IOException e) {
            throw new RuntimeException("Cannot create temp recording file", e);
        }
    }

    @Override
    public final void deleteTempMessage(TempMessage message) {
        FileUtils.deleteQuietly(new File(message.getTempPath()));
    }

    @Override
    public final void storeInInbox(User destUser, TempMessage message) {
        store(destUser, Folder.INBOX, message, VOICEMAIL_SUBJECT);
    }

    @Override
    public final void store(User destUser, Folder folder, TempMessage message, String subject) {
        // Not this one, just delete any temp file
        if (!message.isToBeStored()) {
            FileUtils.deleteQuietly(new File(message.getTempPath()));
            return;
        }

        if (!message.isStored()) {
            String messageId = nextMessageId(m_mailstoreDirectory + "/..");
            VmMessage savedMessage = saveTempMessageInStorage(
                    destUser,
                    message,
                    createMessageDescriptor(destUser.getUserName(), message, messageId, subject,
                            destUser.getIdentity()), folder, messageId);
            message.setSavedMessageId(messageId);
            message.setStored(true);
            if (savedMessage != null) {
                // Make sure to set folder after the message was correctly saved
                savedMessage.setParentFolder(folder);
                m_emailer.queueVm2Email(destUser, savedMessage);
            }
        }
    }

    @Override
    public final void copyMessage(User destUser, TempMessage message) {
        String newMessageId = nextMessageId(m_mailstoreDirectory + "/..");
        VmMessage savedMessage = copyMessage(newMessageId, destUser, message);
        if (savedMessage != null) {
            // the method applies only to voicemails - so the folder where the message is saved is
            // always INBOX
            savedMessage.setParentFolder(Folder.INBOX);
            m_emailer.queueVm2Email(destUser, savedMessage);
        }
    }

    @Override
    public void forwardMessage(User destUser, VmMessage message, TempMessage comments) {
        String newMessageId = nextMessageId(m_mailstoreDirectory + "/..");
        MessageDescriptor descriptor = createMessageDescriptor(destUser.getUserName(), comments, newMessageId,
                destUser.getIdentity());
        long totalDuration = descriptor.getDurationSecsLong() + message.getDescriptor().getDurationSecsLong();
        descriptor.setDurationSecs(totalDuration);
        descriptor.setSubject("Fwd:Voice Message " + newMessageId);
        VmMessage savedMessage = forwardMessage(message, comments, descriptor, destUser, newMessageId);
        if (savedMessage != null) {
            // the method applies only to voicemails - so the folder where the message is saved is
            // always INBOX
            savedMessage.setParentFolder(Folder.INBOX);
            m_emailer.queueVm2Email(destUser, savedMessage);
        }
        comments.setSavedMessageId(newMessageId);
        comments.setStored(true);
    }

    @Override
    public final void saveActiveGreeting(User user, GreetingType type) {
        // only preferences updated here are the active greeing and that is done
        // via a sipXconfig REST call

        // /sipxconfig/rest/my/mailbox/200/preferences/activegreeting/standard

        RestfulRequest rr = new RestfulRequest(m_configUrl + "/sipxconfig/rest/my/mailbox/" + user.getUserName()
                + "/preferences/activegreeting/", user.getUserName(), m_secret);

        try {
            if (rr.put(type.getId())) {
                LOG.info("Mailbox::writeMailboxPreferences:change Greeting " + user.getUserName()
                        + " greeting changed.");
            }
        } catch (Exception e) {
            LOG.info("Mailbox::writeMailboxPreferences:change Greeting " + user.getUserName() + " failed: "
                    + e.getMessage());
        }
    }

    @Override
    public final boolean changePin(User destUser, String newPin) {
        try {
            // Use sipXconfig's RESTful interface to change the PIN
            RestfulRequest rr = new RestfulRequest(m_configUrl + "/sipxconfig/rest/my/voicemail/pin/",
                    destUser.getUserName(), m_secret);
            return rr.put(newPin);
        } catch (Exception e) {
            LOG.error("Retrieve::voicemailOptions new pin trouble", e);
        }
        return false;
    }

    @Override
    public final boolean manageSpecialMode(User user, boolean enable) {
        try {
            // Use sipXconfig's RESTful interface to change the special mode
            RestfulRequest rr = new RestfulRequest(m_configUrl + "/sipxconfig/rest/auto-attendant/specialmode",
                    user.getUserName(), m_secret);
            if (enable) {
                return rr.put(null);
            } else {
                return rr.delete();
            }
        } catch (Exception e) {
            LOG.error("Retrieve::adminOptions:specialmode trouble", e);
        }
        return false;
    }

    @Override
    public final boolean manageLiveAttendant(String code, boolean enable) {
        try {
            String url = String.format("%s/sipxconfig/rest/auto-attendant/livemode/%s", m_configUrl, code);
            // Use sipXconfig's RESTful interface to change the special mode
            RestfulRequest rr = new RestfulRequest(url, "superadmin", m_secret);
            if (enable) {
                return rr.put(null);
            } else {
                return rr.delete();
            }
        } catch (Exception e) {
            LOG.error("Failure to manage live attendant", e);
        }
        return false;
    }

    @Override
    public void migrate(String path) {
        // do nothing, classes that extends this should provide a mean for migrating voicemails
    }

    private MessageDescriptor createMessageDescriptor(String destUser, TempMessage message, String messageId,
            String subject, String identity) {
        MessageDescriptor descriptor = new MessageDescriptor();
        descriptor.setId(identity);
        descriptor.setFromUri(message.getFromUri());
        descriptor.setDurationSecs(message.getDuration());
        descriptor.setContentLength(message.getContentLength());
        descriptor.setTimestamp(message.getTimestamp());
        descriptor.setSubject(subject + messageId);
        descriptor.setPriority(message.getPriority());
        if (message.getOtherRecipients() != null) {
            for (User recipient : message.getOtherRecipients()) {
                if (!recipient.getUserName().equals(destUser)) {
                    descriptor.addOtherRecipient(recipient.getUserName());
                }
            }
        }
        descriptor.setAudioFormat(getAudioFormat());
        return descriptor;
    }

    // Creates VOICEMAIL message descriptors only
    private MessageDescriptor createMessageDescriptor(String destUser, TempMessage message, String messageId,
            String identity) {
        return createMessageDescriptor(destUser, message, messageId, VOICEMAIL_SUBJECT, identity);
    }

    /**
     * Generate the next message Id static synchronized as it's machine wide
     * 
     * @param directory which holds the messageid.txt file
     */
    private synchronized String nextMessageId(String directory) {
        File midFile = new File(directory, "messageid.txt");
        String messageIdFilePath = midFile.getPath();
        long numericMessageId;
        String messageId;
        if (!midFile.exists()) {
            numericMessageId = 1;
            String format = m_identity + "%08d";
            messageId = String.format(format, numericMessageId);
            numericMessageId++;
            try {
                FileUtils.writeStringToFile(midFile, String.format(format, numericMessageId));
            } catch (IOException e) {
                LOG.error("Message::nextMessageId cannot write " + messageIdFilePath, e);
                throw new RuntimeException(e);
            }
            return messageId;
        }

        try {
            // The messageid in the file is the NEXT one
            messageId = FileUtils.readFileToString(midFile);
            numericMessageId = Long.parseLong(StringUtils.deleteWhitespace(messageId));
        } catch (IOException e) {
            LOG.error("Message::nextMessageId cannot read " + messageIdFilePath, e);
            throw new RuntimeException(e);
        }
        // Increment message id, store for another day
        numericMessageId++;
        try {
            FileUtils.writeStringToFile(midFile, String.valueOf(numericMessageId));
        } catch (IOException e) {
            LOG.error("Message::nextMessageId cannot write " + messageIdFilePath, e);
            throw new RuntimeException(e);
        }

        return messageId;
    }

    protected Folder getFolderFromName(String name) {
        if (Folder.INBOX.toString().equals(name)) {
            return Folder.INBOX;
        }
        if (Folder.SAVED.toString().equals(name)) {
            return Folder.SAVED;
        }
        if (Folder.DELETED.toString().equals(name)) {
            return Folder.DELETED;
        }
        if (Folder.CONFERENCE.toString().equals(name)) {
            return Folder.CONFERENCE;
        }
        return null;
    }

    protected String getGreetingTypeName(GreetingType type) {
        return getGreetingTypeName(type, m_audioFormat);
    }

    protected String getAltGreetingTypeName(GreetingType type) {
        return getGreetingTypeName(type, m_altAudioFormat);
    }

    private String getGreetingTypeName(GreetingType type, String audioFormat) {
        switch (type) {
        case STANDARD:
            return String.format("standard.%s", audioFormat);
        case OUT_OF_OFFICE:
            return String.format("outofoffice.%s", audioFormat);
        case EXTENDED_ABSENCE:
            return String.format("extendedabs.%s", audioFormat);
        default:
            return null;
        }
    }

    protected void concatAudio(File newFile, File orig1, File orig2) throws Exception {
        if (getAudioFormat().equals("wav")) {
            concatWavAudio(newFile, orig1, orig2);
        } if (getAudioFormat().equals("mp3")) {
            concatMP3Audio(newFile, orig1, orig2);
        }
    }

    private void concatWavAudio(File newFile, File orig1, File orig2) throws Exception {
        String operation = "dunno";
        AudioInputStream clip1 = null;
        AudioInputStream clip2 = null;
        AudioInputStream concatStream = null;
        try {
            operation = "getting AudioInputStream from " + orig1.getPath();
            clip1 = AudioSystem.getAudioInputStream(orig1);
            operation = "getting AudioInputStream from " + orig2.getPath();
            clip2 = AudioSystem.getAudioInputStream(orig2);

            operation = "building SequnceInputStream";
            concatStream = new AudioInputStream(new SequenceInputStream(clip1, clip2), clip1.getFormat(),
                    clip1.getFrameLength() + clip2.getFrameLength());

            operation = "writing SequnceInputStream to " + newFile.getPath();
            AudioSystem.write(concatStream, AudioFileFormat.Type.WAVE, newFile);
            LOG.info("VmMessage::concatAudio created combined file " + newFile.getPath());
        } catch (Exception e) {
            String trouble = "VmMessage::concatAudio Problem while " + operation;
            throw new Exception(trouble, e);
        } finally {
            IOUtils.closeQuietly(clip1);
            IOUtils.closeQuietly(clip2);
            IOUtils.closeQuietly(concatStream);
        }
    }

    private void concatMP3Audio(File newFile, File orig1, File orig2) throws Exception {
        FileInputStream comment = new FileInputStream(orig1);
        FileInputStream original = new FileInputStream(orig2);
        SequenceInputStream concatStream = new SequenceInputStream(comment, original);
        FileOutputStream concat = new FileOutputStream(newFile);
        try {

            int temp;

            while( ( temp = concatStream.read() ) != -1)
            {
                concat.write(temp);
            }
        } catch (Exception ex) {
            throw new Exception("VmMessage::concatAudio Problem while writting MP3 file", ex);
        } finally {
            IOUtils.closeQuietly(concat);
            IOUtils.closeQuietly(concatStream);
            IOUtils.closeQuietly(comment);
            IOUtils.closeQuietly(original);
        }
    }

    public String getNameFile() {
        return getNameFile(m_audioFormat);
    }

    public String getAltNameFile() {
        return getNameFile(m_altAudioFormat);
    }

    private String getNameFile(String audioFormat) {
        return String.format("name.%s", audioFormat);
    }

    public String getPromptFile() {
        return String.format("customautoattendant-%d.%s", System.currentTimeMillis() / 1000, m_audioFormat);
    }

    public void setMailstoreDirectory(String dir) {
        m_mailstoreDirectory = dir;
    }

    public void setPromptsDirectory(String dir) {
        m_promptsDirectory = dir;
    }

    public void setOperatorAddr(String operatorAddr) {
        m_operatorAddr = operatorAddr;
    }

    public void setConfigUrl(String configUrl) {
        m_configUrl = configUrl;
    }

    public void setSecret(String secret) {
        m_secret = secret;
    }

    public void setEmailer(Emailer emailer) {
        m_emailer = emailer;
    }

    public void setMwiManager(Mwi mwiManager) {
        m_mwi = mwiManager;
    }

    public void setIvrIdentity(String identity) {
        m_identity = identity;
    }

    public void setAudioFormat(String format) {
        m_audioFormat = format;
        if (m_audioFormat.equals("mp3")) {
            m_altAudioFormat = "wav";
        } else {
            m_altAudioFormat = "mp3";
        }
    }

    public String getAudioFormat() {
        return m_audioFormat;
    }

    public String getAltAudioFormat() {
        return m_altAudioFormat;
    }

}
