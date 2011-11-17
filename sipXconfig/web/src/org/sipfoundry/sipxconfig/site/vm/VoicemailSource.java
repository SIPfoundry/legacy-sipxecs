/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.vm;

import java.io.Serializable;
import java.util.Collection;
import java.util.Map;

import org.sipfoundry.sipxconfig.vm.Voicemail;

public class VoicemailSource {
    private Map<Serializable, Voicemail> m_voicemails;

    public VoicemailSource(Map<Serializable, Voicemail> voicemails) {
        m_voicemails = voicemails;
    }

    public Voicemail getVoicemail(Serializable voicemailId) {
        return m_voicemails.get(voicemailId);
    }

    public Collection<Voicemail> getVoicemails() {
        return m_voicemails.values();
    }

    public static Serializable getVoicemailId(Voicemail vm) {
        if (vm != null) {
            return vm.getUserId() + '/' + vm.getFolderId() + '/' + vm.getMessageId();
        }
        return -1;
    }
}
