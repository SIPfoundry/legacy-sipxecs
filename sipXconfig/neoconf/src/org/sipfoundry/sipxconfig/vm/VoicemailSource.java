/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.vm;

import java.io.File;
import java.io.Serializable;

public class VoicemailSource {
    private File m_mailstore;

    public VoicemailSource(File mailstore) {
        m_mailstore = mailstore;
    }

    public Voicemail getVoicemail(Serializable voicemailId) {
        String[] ids = decodeVoicemailId(voicemailId);
        return new Voicemail(m_mailstore, ids[0], ids[1], ids[2]);
    }

    public Serializable getVoicemailId(Voicemail vm) {
        return vm.getUserId() + '/' + vm.getFolderId() + '/' + vm.getMessageId();
    }

    public static String[] decodeVoicemailId(Object primaryKey) {
        String[] ids = primaryKey.toString().split(String.valueOf('/'));
        return ids;
    }
}
