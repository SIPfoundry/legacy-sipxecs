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

public class VmMessage {
    private File m_audioFile;
    private MessageDescriptor m_descriptor;
    private String m_username;
    private String m_messageId;
    private Folder m_folder;
    private boolean m_unheard;
    private boolean m_urgent;

    public VmMessage(String messageId, String username, File audioFile, MessageDescriptor descriptor, Folder folder,
            boolean unheard, boolean urgent) {
        m_audioFile = audioFile;
        m_descriptor = descriptor;
        m_username = username;
        m_messageId = messageId;
        m_folder = folder;
        m_unheard = unheard;
        m_urgent = urgent;
    }

    protected VmMessage(String messageId, File audioFile, MessageDescriptor descriptor, boolean urgent) {
        m_audioFile = audioFile;
        m_descriptor = descriptor;
        m_messageId = messageId;
        m_urgent = urgent;
    }

    public File getAudioFile() {
        return m_audioFile;
    }

    public MessageDescriptor getDescriptor() {
        return m_descriptor;
    }

    public String getUserName() {
        return m_username;
    }

    public String getMessageId() {
        return m_messageId;
    }

    public Folder getParentFolder() {
        return m_folder;
    }

    public boolean isUnHeard() {
        return m_unheard;
    }

    public boolean isUrgent() {
        return m_urgent;
    }

    public void cleanup() {
        
    }

}
