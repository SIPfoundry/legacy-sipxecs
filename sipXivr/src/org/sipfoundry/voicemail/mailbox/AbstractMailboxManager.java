/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.voicemail.mailbox;

import java.io.File;
import java.io.IOException;
import java.io.SequenceInputStream;

import javax.sound.sampled.AudioFileFormat;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;

import org.apache.commons.io.FileUtils;
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

    protected abstract VmMessage saveTempMessageInStorage(User destUser, TempMessage message,
            MessageDescriptor descriptor, String messageId);

    protected abstract File getTempFolder(String username);

    protected abstract VmMessage copyMessage(String newMessageId, User destUser, TempMessage message);

    protected abstract VmMessage forwardMessage(VmMessage originalMessage, TempMessage comments,
            MessageDescriptor descriptor, User destUser, String newMessageId);

    protected abstract MailboxPreferences saveActiveGreetingOnStorage(User user, GreetingType type);

    protected abstract void writeMailboxFile(User user, String fileName, String content);

    @Override
    public final TempMessage createTempMessage(String username, String fromUri, boolean audio) {
        try {
            String audioPath = null;
            if (audio) {
                File wavFile = File.createTempFile("temp_recording_", ".wav", getTempFolder(username));
                audioPath = wavFile.getPath();
            }
            return new TempMessage(username, audioPath, fromUri, Priority.NORMAL, null);
        } catch (IOException e) {
            throw new RuntimeException("Cannot create temp recording file", e);
        }
    }

    @Override
    public final void deleteTempMessage(TempMessage message) {
        FileUtils.deleteQuietly(new File(message.getTempWavPath()));
    }

    @Override
    public final void storeInInbox(User destUser, TempMessage message) {
        // Not this one, just delete any temp file
        if (!message.isToBeStored()) {
            FileUtils.deleteQuietly(new File(message.getTempWavPath()));
            return;
        }

        if (!message.isStored()) {
            String messageId = nextMessageId(m_mailstoreDirectory + "/..");
            VmMessage savedMessage = saveTempMessageInStorage(destUser, message,
                    createMessageDescriptor(destUser.getUserName(), message, messageId, destUser.getIdentity()),
                    messageId);
            message.setSavedMessageId(messageId);
            message.setStored(true);
            if (savedMessage != null) {
                m_emailer.queueVm2Email(destUser, savedMessage);
            }
        }
    }

    @Override
    public final void copyMessage(User destUser, TempMessage message) {
        String newMessageId = nextMessageId(m_mailstoreDirectory + "/..");
        VmMessage savedMessage = copyMessage(newMessageId, destUser, message);
        if (savedMessage != null) {
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
            m_emailer.queueVm2Email(destUser, savedMessage);
        }
        comments.setSavedMessageId(newMessageId);
        comments.setStored(true);
    }

    @Override
    public final void saveActiveGreeting(User user, GreetingType type) {
        writeConfigMailboxPreferences(user, saveActiveGreetingOnStorage(user, type));
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
    public final void savePersonalAttendant(User user, String content) {
        writeMailboxFile(user, "PersonalAttendant.properties", content);
    }

    @Override
    public final void saveMailboxPrefs(User user, String content) {
        writeMailboxFile(user, "mailboxprefs.xml", content);
    }

    @Override
    public final void saveDistributionList(User user, String content) {
        writeMailboxFile(user, "distribution.xml", content);
    }

    private MessageDescriptor createMessageDescriptor(String destUser, TempMessage message, String messageId,
            String identity) {
        MessageDescriptor descriptor = new MessageDescriptor();
        descriptor.setId(identity);
        descriptor.setFromUri(message.getFromUri());
        descriptor.setDurationSecs(message.getDuration());
        descriptor.setTimestamp(message.getTimestamp());
        descriptor.setSubject("Voice Message " + messageId);
        descriptor.setPriority(message.getPriority());
        if (message.getOtherRecipients() != null) {
            for (User recipient : message.getOtherRecipients()) {
                if (!recipient.getUserName().equals(destUser)) {
                    descriptor.addOtherRecipient(recipient.getUserName());
                }
            }
        }
        return descriptor;
    }

    /**
     * Generate the next message Id static synchronized as it's machine wide
     * 
     * @param directory which holds the messageid.txt file
     */
    private static synchronized String nextMessageId(String directory) {
        long numericMessageId = 1;
        String format = "%08d";
        String messageId = String.format(format, numericMessageId);

        // messageid.txt file is (hopefully) in the directory
        File midFile = new File(directory, "messageid.txt");
        String messageIdFilePath = midFile.getPath();
        if (midFile.exists()) {
            try {
                // The messageid in the file is the NEXT one
                messageId = FileUtils.readFileToString(midFile);

                // on older systems, this messageid.txt file may have a newline
                // character in it.. need to strip if off
                if (messageId.endsWith("\n")) {
                    messageId = messageId.substring(0, 8);
                }

                numericMessageId = Long.parseLong(messageId);
            } catch (IOException e) {
                LOG.error("Message::nextMessageId cannot read " + messageIdFilePath, e);
                throw new RuntimeException(e);
            }
        }
        // Increment message id, store for another day
        numericMessageId++;
        try {
            FileUtils.writeStringToFile(midFile, String.format(format, numericMessageId));
        } catch (IOException e) {
            LOG.error("Message::nextMessageId cannot write " + messageIdFilePath, e);
            throw new RuntimeException(e);
        }

        return messageId;
    }

    private void writeConfigMailboxPreferences(User user, MailboxPreferences prefs) {
        // only preferences updated here are the active greeing and that is done
        // via a sipXconfig REST call

        // /sipxconfig/rest/my/mailbox/200/preferences/activegreeting/standard

        RestfulRequest rr = new RestfulRequest(m_configUrl + "/sipxconfig/rest/my/mailbox/" + user.getUserName()
                + "/preferences/activegreeting/", user.getUserName(), m_secret);

        try {
            if (rr.put(prefs.getActiveGreeting().getActiveGreeting())) {
                LOG.info("Mailbox::writeMailboxPreferences:change Greeting " + user.getUserName()
                        + " greeting changed.");
            }
        } catch (Exception e) {
            LOG.info("Mailbox::writeMailboxPreferences:change Greeting " + user.getUserName() + " failed: "
                    + e.getMessage());
        }
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
        switch (type) {
        case STANDARD:
            return "standard.wav";
        case OUT_OF_OFFICE:
            return "outofoffice.wav";
        case EXTENDED_ABSENCE:
            return "extendedabs.wav";
        default:
            return null;
        }
    }

    protected void concatAudio(File newFile, File orig1, File orig2) throws Exception {
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
            clip1.close();
            clip2.close();
            concatStream.close();
        }
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

}
