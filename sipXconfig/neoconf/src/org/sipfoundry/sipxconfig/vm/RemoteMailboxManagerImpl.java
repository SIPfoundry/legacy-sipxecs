/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.vm;

import java.util.ArrayList;
import java.util.List;

import javax.xml.transform.Source;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.springframework.web.client.RestClientException;
import org.springframework.web.client.RestTemplate;
import org.springframework.xml.xpath.NodeMapper;
import org.springframework.xml.xpath.XPathOperations;
import org.w3c.dom.Element;
import org.w3c.dom.Node;

public class RemoteMailboxManagerImpl extends AbstractMailboxManager implements MailboxManager {
    private static final Log LOG = LogFactory.getLog(AbstractMailboxManager.class);

    private static final String GET_VOICEMAILS_URL = "/mailbox/{userId}/{folder}";
    private static final String GET_VOICEMAIL_URL = "/mailbox/{userId}/{folder}/{messageId}";
    private static final String DELETE_MAILBOX_URL = "/mailbox/{userId}/delete";
    private static final String RENAME_MAILBOX_URL = "/mailbox/{userId}/rename/{oldUserId}";
    private static final String MESSAGE_READ_URL = "/mailbox/{userId}/message/{messageId}/heard";
    private static final String MOVE_MSG = "/mailbox/{userId}/"
            + "message/{messageId}/move/{destination}";
    private static final String DEL_MSG_URL = "/mailbox/{userId}/message/{messageId}/delete";
    private static final String SAVE_SUBJECT = "/mailbox/{userId}/message/{messageId}/subject";
    private static final String LINE_BREAK = "\n";
    private XPathOperations m_xPathTemplate;
    private RestTemplate m_restTemplate;
    private Address m_lastGoodIvrNode;

    @Override
    public boolean isEnabled() {
        return true;
    }

    @Override
    public List<Voicemail> getVoicemail(final String userId, final String folder) {
        return retrieveVoicemails(GET_VOICEMAILS_URL, userId, folder);
    }

    @Override
    public Voicemail getVoicemail(final String userId, final String folder, final String messageId) {
        List<Voicemail> voicemails = retrieveVoicemails(GET_VOICEMAIL_URL, userId, folder, messageId);
        if (voicemails != null) {
            return voicemails.get(0);
        }
        return null;
    }

    private List<Voicemail> retrieveVoicemails(String url, final Object... urlVariables) {
        Address lastGoodIvrNode = getLastGoodIvrNode();
        List<Address> ivrRestAddresses = null;
        if (lastGoodIvrNode != null) {
            ivrRestAddresses = new ArrayList<Address>();
            ivrRestAddresses.add(lastGoodIvrNode);
            try {
                return retrieveVoicemails(ivrRestAddresses, url, urlVariables);
            } catch (UserException ex) {
                //do not throw exception because we have to iterate again through all nodes
                LOG.warn("Cannot connect to " + lastGoodIvrNode + " for reason: " + ex.getMessage());
            }
        }
        ivrRestAddresses = getAddressManager().getAddresses((Ivr.REST_API));
        return retrieveVoicemails(ivrRestAddresses, url, urlVariables);
    }

    private List<Voicemail> retrieveVoicemails(List<Address> ivrRestAddresses,
            String url, final Object... urlVariables) {
        StringBuilder failedAddresses = new StringBuilder();
        StringBuilder messages = new StringBuilder();
        for (Address address : ivrRestAddresses) {
            try {
                Source voicemails = m_restTemplate.getForObject(address + url, Source.class, urlVariables);
                List<Voicemail> voicemailList = m_xPathTemplate.evaluate("//message", voicemails,
                    new NodeMapper<Voicemail>() {
                        @Override
                        public Voicemail mapNode(Node node, int pos) {
                            return new RemoteVoicemail((Element) node, (String) urlVariables[0],
                                    (String) urlVariables[1]);
                        }
                    });
                setLastGoodIvrNode(address);
                return voicemailList;
            } catch (RestClientException ex) {
                failedAddresses.append(address).append(LINE_BREAK);
                messages.append(ex.getMessage()).append(LINE_BREAK);
            }
        }
        throw createUserException(failedAddresses.toString(), messages.toString());
    }

    @Override
    public void deleteMailbox(String userId) {
        putWithFallback(DELETE_MAILBOX_URL, null, userId);
    }

    @Override
    public void renameMailbox(String oldUserId, String newUserId) {
        putWithFallback(RENAME_MAILBOX_URL, null, newUserId, oldUserId);
    }

    @Override
    public void markRead(String userId, String messageId) {
        putWithFallback(MESSAGE_READ_URL, null, userId, messageId);
    }

    @Override
    public void move(String userId, Voicemail voicemail, String destinationFolderId) {
        putWithFallback(MOVE_MSG, null, userId, voicemail.getMessageId(),
                destinationFolderId);
    }

    @Override
    public void delete(String userId, Voicemail voicemail) {
        putWithFallback(DEL_MSG_URL, null, userId, voicemail.getMessageId());
    }

    @Override
    public void save(Voicemail voicemail) {
        putWithFallback(SAVE_SUBJECT, voicemail.getSubject(),
                voicemail.getUserId(), voicemail.getMessageId());
    }

    public void setXpathTemplate(XPathOperations xpathTemplate) {
        m_xPathTemplate = xpathTemplate;
    }

    public void setRestTemplate(RestTemplate restTemplate) {
        m_restTemplate = restTemplate;
    }

    private void putWithFallback(String relativeUri, Object request, Object... params) {
        Address lastGoodIvrNode = getLastGoodIvrNode();
        List<Address> ivrRestAddresses = null;
        if (lastGoodIvrNode != null) {
            ivrRestAddresses = new ArrayList<Address>();
            ivrRestAddresses.add(lastGoodIvrNode);
            try {
                putWithFallback(ivrRestAddresses, relativeUri, request, params);
                return;
            } catch (UserException ex) {
                //do no throw exception because we have to iterate again through all nodes
                LOG.warn("Cannot connect to node " + lastGoodIvrNode + " for following reason: " + ex.getMessage());
            }
        }
        ivrRestAddresses = getAddressManager().getAddresses((Ivr.REST_API));
        putWithFallback(ivrRestAddresses, relativeUri, request, params);
    }

    private void putWithFallback(List<Address> addresses, String relativeUri, Object request, Object... params) {
        boolean success = false;
        StringBuilder failedAddresses = new StringBuilder();
        StringBuilder messages = new StringBuilder();
        for (Address address : addresses) {
            try {
                m_restTemplate.put(address + relativeUri, request, params);
                success = true;
                setLastGoodIvrNode(address);
                break;
            } catch (RestClientException ex) {
                failedAddresses.append(address).append(LINE_BREAK);
                messages.append(ex.getMessage()).append(LINE_BREAK);
            }
        }
        if (!success) {
            throw createUserException(failedAddresses.toString(), messages.toString());
        }
    }

    private UserException createUserException(String addresses, String messages) {
        return new UserException("&error.ivr.server", addresses, messages);
    }

    public Address getLastGoodIvrNode() {
        return m_lastGoodIvrNode;
    }

    public void setLastGoodIvrNode(Address lastGoodIvrNode) {
        m_lastGoodIvrNode = lastGoodIvrNode;
    }
}
