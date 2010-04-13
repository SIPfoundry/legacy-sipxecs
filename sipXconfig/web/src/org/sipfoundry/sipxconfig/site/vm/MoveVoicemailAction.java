/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.vm;

import java.io.Serializable;
import java.util.Collection;

import org.apache.tapestry.IComponent;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.site.setting.BulkGroupAction;
import org.sipfoundry.sipxconfig.vm.Mailbox;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.Voicemail;
import org.sipfoundry.sipxconfig.vm.VoicemailSource;

public class MoveVoicemailAction extends BulkGroupAction {
    private String m_folderId;
    private String m_folderLabel;
    private VoicemailSource m_source;
    private MailboxManager m_mgr;

    public MoveVoicemailAction(MailboxManager mgr, VoicemailSource source, String folderLabel, String folderId) {
        super(null);
        m_mgr = mgr;
        m_folderLabel = folderLabel;
        m_folderId = folderId;
        m_source = source;
    }

    public void actionTriggered(IComponent arg0, IRequestCycle arg1) {
        for (Serializable id : (Collection<Serializable>) getIds()) {
            Voicemail vm = m_source.getVoicemail(id);
            Mailbox mbox = m_mgr.getMailbox(vm.getUserId());
            m_mgr.move(mbox, vm, m_folderId);
        }
    }

    public String getLabel(Object option, int index) {
        return m_folderLabel;
    }

    public Object getValue(Object option, int index) {
        return this;
    }

    public String squeezeOption(Object option, int index) {
        return getClass().getName() + m_folderId;
    }
}
