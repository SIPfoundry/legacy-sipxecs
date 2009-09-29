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

import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.engine.ILink;
import org.apache.tapestry.services.ServiceConstants;
import org.apache.tapestry.util.ContentType;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.FileService;
import org.sipfoundry.sipxconfig.vm.Mailbox;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.Voicemail;

/**
 * Send voicemail file to user and mark it as read
 */
public class PlayVoicemailService extends FileService {
    public static final String SERVICE_NAME = "playvoicemail";

    private MailboxManager m_mailboxManager;

    private CoreContext m_coreContext;

    public String getName() {
        return SERVICE_NAME;
    }

    /**
     * The only parameter is the service parameters[dirName, fileName]
     */
    public void service(IRequestCycle cycle) throws IOException {
        Mailbox mailbox = m_mailboxManager.getMailbox(getUserName());

        // HACK: apparently cycle.getListenerParameters() returns null, but this
        // method seems to work just fine. Tapestry bug?
        Object[] listenerParameters = getLinkFactory().extractListenerParameters(cycle);

        Info info = new Info(listenerParameters);
        cycle.getParameters(ServiceConstants.PARAMETER);
        Voicemail voicemail = mailbox.getVoicemail(info.getFolderId(), info.getMessageId());
        File file = voicemail.getMediaFile();
        sendFile(file, info.getDigest(), new ContentType("audio/x-wav"));

        m_mailboxManager.markRead(mailbox, voicemail);
    }

    public static class Info {
        private String m_messageId;
        private String m_folderId;
        private String m_digest;
        public Info(Object[] serviceParameters) {
            m_folderId = (String) serviceParameters[0];
            m_messageId = (String) serviceParameters[1];
            m_digest = (String) serviceParameters[2];
        }
        public Info(String folderId, String messageId) {
            m_folderId = folderId;
            m_messageId = messageId;
        }
        public Object[] getServiceParameters() {
            return new Object[] {
                    getFolderId(),
                    getMessageId(),
                    getDigest()
            };
        }
        public String getFolderId() {
            return m_folderId;
        }
        public String getMessageId() {
            return m_messageId;
        }
        public String getDigest() {
            return m_digest;
        }
        public void setDigest(String digest) {
            m_digest = digest;
        }
    }

    public ILink getLink(boolean post, Object parameter) {
        Integer userId = requireUserId();

        Info info = (Info) ((Object[]) parameter)[0];
        Mailbox mb = m_mailboxManager.getMailbox(getUserName());
        Voicemail vm = mb.getVoicemail(info.getFolderId(), info.getMessageId());
        String digest = getDigestSource().getDigestForResource(userId, vm.getMediaFile().getAbsolutePath());
        info.setDigest(digest);

        Map parameters = new HashMap();

        parameters.put(ServiceConstants.SERVICE, getName());
        parameters.put(ServiceConstants.PARAMETER , info.getServiceParameters());

        return getLinkFactory().constructLink(this, post, parameters, false);
    }

    public void setMailboxManager(MailboxManager mailboxManager) {
        m_mailboxManager = mailboxManager;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public String getUserName() {
        Integer userId = getUserId();
        if (userId == null) {
            return null;
        }

        User user = m_coreContext.loadUser(userId);
        return user.getUserName();
    }
}
