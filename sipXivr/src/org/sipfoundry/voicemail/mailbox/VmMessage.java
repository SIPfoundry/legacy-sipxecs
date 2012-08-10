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

    public void setParentFolder(Folder folder) {
        m_folder = folder;
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
