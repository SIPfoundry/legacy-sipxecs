/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.vm;

import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringWriter;
import java.util.Collection;
import java.util.List;

import javax.xml.transform.Source;
import javax.xml.transform.stream.StreamSource;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.vm.attendant.PersonalAttendant;
import org.springframework.web.client.RestClientException;
import org.springframework.web.client.RestTemplate;
import org.springframework.xml.xpath.NodeMapper;
import org.springframework.xml.xpath.XPathOperations;
import org.w3c.dom.Element;
import org.w3c.dom.Node;

public class RemoteMailboxManagerImpl extends AbstractMailboxManager implements MailboxManager {
    private static final String GET_VOICEMAILS_URL = "https://{host}:{port}/mailbox/{userId}/{folder}";
    private static final String GET_VOICEMAIL_URL = "https://{host}:{port}/mailbox/{userId}/{folder}/{messageId}";
    private static final String DELETE_MAILBOX_URL = "https://{host}:{port}/mailbox/{userId}/delete";
    private static final String RENAME_MAILBOX_URL = "https://{host}:{port}/mailbox/{userId}/rename/{oldUserId}";
    private static final String DISTRIBUTION_URL = "https://{host}:{port}/mailbox/{userId}/distribution";
    private static final String MESSAGE_READ_URL = "https://{host}:{port}/mailbox/{userId}/message/{messageId}/heard";
    private static final String MOVE_MSG = "https://{host}:{port}/mailbox/{userId}/"
            + "message/{messageId}/move/{destination}";
    private static final String DEL_MSG_URL = "https://{host}:{port}/mailbox/{userId}/message/{messageId}/delete";
    private static final String SAVE_SUBJECT = "https://{host}:{port}/mailbox/{userId}/message/{messageId}/subject";
    private static final String PA_URL = "https://{host}:{port}/mailbox/{userId}/personalattendant";
    private static final String ACTIVE_GREETING_URL = "https://{host}:{port}/mailbox/{userId}/activegreeting";
    private XPathOperations m_xPathTemplate;
    private RestTemplate m_restTemplate;

    @Override
    public boolean isEnabled() {
        return true;
    }

    @Override
    public List<Voicemail> getVoicemail(final String userId, final String folder) {
        return retrieveVoicemails(GET_VOICEMAILS_URL, userId, folder);
    }

    @Override
    public Voicemail getVoicemail(final String userId, final String folder, String messageId) {
        List<Voicemail> voicemails = retrieveVoicemails(GET_VOICEMAIL_URL, userId, folder);
        if (voicemails != null) {
            return voicemails.get(0);
        }
        return null;
    }

    private List<Voicemail> retrieveVoicemails(String url, final String userId, final String folder) {
        try {
            Source voicemails = m_restTemplate.getForObject(url, Source.class, getHost(), getPort(), userId, folder);
            List<Voicemail> voicemailList = m_xPathTemplate.evaluate("//message", voicemails,
                    new NodeMapper<Voicemail>() {
                        @Override
                        public Voicemail mapNode(Node node, int pos) {
                            return new RemoteVoicemail((Element) node, userId, folder);
                        }
                    });
            return voicemailList;
        } catch (RestClientException ex) {
            throw createUserException(ex.getMessage());
        }
    }

    @Override
    public void deleteMailbox(String userId) {
        put(DELETE_MAILBOX_URL, null, getHost(), getPort(), userId);
    }

    @Override
    public void renameMailbox(String oldUserId, String newUserId) {
        put(RENAME_MAILBOX_URL, null, getHost(), getPort(), newUserId, oldUserId);
    }

    @Override
    public void saveDistributionLists(String userId, DistributionList[] lists) {
        Collection<String> aliases = DistributionList.getUniqueExtensions(lists);
        getCoreContext().checkForValidExtensions(aliases, PermissionName.VOICEMAIL);
        StringWriter request = new StringWriter();
        getDistributionListsWriter().writeObject(lists, request);
        try {
            m_restTemplate.put(DISTRIBUTION_URL, request.toString(), getHost(), getPort(), userId);
        } catch (RestClientException ex) {
            throw createUserException(ex.getMessage());
        }
    }

    @Override
    public DistributionList[] loadDistributionLists(String userId) {
        try {
            StreamSource distributions = m_restTemplate.getForObject(DISTRIBUTION_URL, StreamSource.class,
                    getHost(), getPort(), userId);
            Reader reader = new InputStreamReader(distributions.getInputStream());
            return getDistributionListsReader().readObject(reader);
        } catch (RestClientException ex) {
            throw createUserException(ex.getMessage());
        }
    }

    @Override
    public void markRead(String userId, String messageId) {
        put(MESSAGE_READ_URL, null, getHost(), getPort(), userId, messageId);
    }

    @Override
    public void move(String userId, Voicemail voicemail, String destinationFolderId) {
        put(MOVE_MSG, null, getHost(), getPort(), userId, voicemail.getMessageId(), destinationFolderId);
    }

    @Override
    public void delete(String userId, Voicemail voicemail) {
        put(DEL_MSG_URL, null, getHost(), getPort(), userId, voicemail.getMessageId());
    }

    @Override
    public void save(Voicemail voicemail) {
        put(SAVE_SUBJECT, voicemail.getSubject(), getHost(), getPort(), voicemail.getUserId(),
                voicemail.getMessageId());
    }

    @Override
    public void writePersonalAttendant(PersonalAttendant pa) {
        StringWriter request = new StringWriter();
        getPersonalAttendantWriter().write(pa, request);
        put(PA_URL, request.toString(), getHost(), getPort(), pa.getUser().getUserName());
    }

    @Override
    public void writePreferencesFile(User user) {
        StringWriter request = new StringWriter();
        getMailboxPreferencesWriter().writeObject(new MailboxPreferences(user), request);
        put(ACTIVE_GREETING_URL, request.toString(), getHost(), getPort(), user.getUserName());
    }

    public void setXpathTemplate(XPathOperations xpathTemplate) {
        m_xPathTemplate = xpathTemplate;
    }

    public void setRestTemplate(RestTemplate restTemplate) {
        m_restTemplate = restTemplate;
    }

    private void put(String uri, Object request, Object... params) {
        try {
            m_restTemplate.put(uri, request, params);
        } catch (RestClientException ex) {
            throw createUserException(ex.getMessage());
        }
    }

    private UserException createUserException(String message) {
        return new UserException("&error.ivr.server", getHost(), message);
    }

}
