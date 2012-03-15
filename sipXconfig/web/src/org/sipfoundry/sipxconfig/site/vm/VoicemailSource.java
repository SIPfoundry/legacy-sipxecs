/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
