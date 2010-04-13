/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.rest;

import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.restlet.Context;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.FileRepresentation;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.vm.Mailbox;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.Voicemail;
import org.springframework.beans.factory.annotation.Required;

import static org.restlet.data.MediaType.AUDIO_WAV;


public class VoicemailResourceDownload extends UserResource {

    private static final Log LOG = LogFactory.getLog(PhonesResource.class);

    private String m_folder;
    private String m_messsageId;
    private MailboxManager m_mailboxManager;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        m_folder = (String) getRequest().getAttributes().get("folder");
        m_messsageId = (String) getRequest().getAttributes().get("messageId");

        getVariants().add(new Variant(AUDIO_WAV));
    }

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        Mailbox mailbox = m_mailboxManager.getMailbox(getUser().getUserName());
        List<Voicemail> voicemails = m_mailboxManager.getVoicemail(mailbox, m_folder);

        Representation representation = null;
        /*
         * <HACK> -----------
         * A .wav suffix may be added at the end of the url to correctly associate the file
         * as a .wav file, when downloading it through the <audio/> controller in FF 3.5
         *
         */

        if (m_messsageId.endsWith(".wav")) {
            m_messsageId = m_messsageId.substring(0, (m_messsageId.length() - 4));
        }

        /*
         * </HACK>
         */

        for (Voicemail voicemail : voicemails) {
            if (voicemail.getMessageId().equals(m_messsageId)) {
                representation = new FileRepresentation(voicemail.getMediaFile(), AUDIO_WAV);
                representation.setDownloadable(true);
                try {
                    m_mailboxManager.markRead(mailbox, voicemail);
                } catch (UserException ex) {
                    LOG.error("Failed to mark voicemail as read", ex);
                }
            }
        }
        return representation;
    }

    @Required
    public void setMailboxManager(MailboxManager mailboxManager) {
        m_mailboxManager = mailboxManager;
    }

}
