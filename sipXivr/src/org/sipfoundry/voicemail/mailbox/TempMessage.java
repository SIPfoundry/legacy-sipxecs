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

import java.io.EOFException;
import java.io.File;
import java.util.List;

import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;

import org.apache.log4j.Logger;
import org.jaudiotagger.audio.AudioFile;
import org.jaudiotagger.audio.AudioFileIO;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.voicemail.mailbox.MessageDescriptor.Priority;

public class TempMessage {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private String m_fromUri;
    private Priority m_priority;
    private List<User> m_otherRecipients;
    private String m_tempPath;
    private boolean m_isToBeStored = true;
    private long m_duration = 0L;
    private long m_timestamp;
    private boolean m_stored = false;
    private String m_currentUser;
    private String m_savedMessageId;

    public TempMessage(String username, String tempPath, String fromUri, Priority priority, List<User> otherRecipients) {
        m_tempPath = tempPath;
        m_fromUri = fromUri;
        m_priority = priority;
        m_otherRecipients = otherRecipients;
        m_timestamp = System.currentTimeMillis();
        m_currentUser = username;
    }

    public Priority getPriority() {
        return m_priority;
    }

    public void togglePriority() {
        if (m_priority == Priority.NORMAL) {
            m_priority = Priority.URGENT;
        } else {
            m_priority = Priority.NORMAL;
        }
    }

    public String getFromUri() {
        return m_fromUri;
    }

    public List<User> getOtherRecipients() {
        return m_otherRecipients;
    }

    public boolean isToBeStored() {
        return m_isToBeStored;
    }

    public void setIsToBeStored(boolean isToBeStored) {
        m_isToBeStored = isToBeStored;
    }

    public long getDuration() {
        // Calculate the duration (in seconds) from the Wav file
        if (m_tempPath != null) {
            File audioFile = new File(m_tempPath);
            if (audioFile != null) {
                if (audioFile.getName().endsWith("mp3")) {
                    extractMp3Duration(audioFile);
                } else {
                    extractWavDuration(audioFile);
                }
            }
        }
        return m_duration;
    }

    private void extractWavDuration(File audioFile) {
        try {
            AudioInputStream ais = AudioSystem.getAudioInputStream(audioFile);
            float secs = ais.getFrameLength() / ais.getFormat().getFrameRate();
            m_duration = Math.round(secs); // Round up.
        } catch (EOFException e) {
            m_duration = 0;
        } catch (Exception e) {
            String trouble = "Message::getDuration Problem determining duration of " + m_tempPath;
            LOG.error(trouble, e);
            throw new RuntimeException(trouble, e);
        }
    }

    private void extractMp3Duration(File audioFile) {
        try {
            AudioFile mp3File = AudioFileIO.read(audioFile);
            m_duration = mp3File.getAudioHeader().getTrackLength();
        } catch (Exception e) {
            String trouble = "Message::getDuration Problem determining duration of " + m_tempPath;
            LOG.error(trouble, e);
            throw new RuntimeException(trouble, e);
        }
    }

    public long getTimestamp() {
        return m_timestamp;
    }

    public String getTempPath() {
        return m_tempPath;
    }

    public String getCurrentUser() {
        return m_currentUser;
    }

    public void resetStoredFlag() {
        m_stored = false;
    }

    protected boolean isStored() {
        return m_stored;
    }

    protected void setStored(boolean stored) {
        m_stored = stored;
    }

    protected String getSavedMessageId() {
        return m_savedMessageId;
    }

    protected void setSavedMessageId(String messageId) {
        m_savedMessageId = messageId;
    }

}
