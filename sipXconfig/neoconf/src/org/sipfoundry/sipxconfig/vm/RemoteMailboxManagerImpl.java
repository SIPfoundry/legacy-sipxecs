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

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.net.HttpURLConnection;
import java.util.List;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLSession;
import javax.xml.transform.Source;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.backup.BackupBean;
import org.sipfoundry.sipxconfig.backup.BackupPlan;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigCommands;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.http.client.SimpleClientHttpRequestFactory;
import org.springframework.web.client.RestClientException;
import org.springframework.web.client.RestTemplate;
import org.springframework.xml.xpath.NodeMapper;
import org.springframework.xml.xpath.XPathOperations;
import org.w3c.dom.Element;
import org.w3c.dom.Node;

public class RemoteMailboxManagerImpl extends AbstractMailboxManager implements MailboxManager {
    private static final String GET_VOICEMAILS_URL = "/mailbox/{userId}/{folder}";
    private static final String GET_VOICEMAIL_URL = "/mailbox/{userId}/{folder}/{messageId}";
    private static final String DELETE_MAILBOX_URL = "/mailbox/{userId}/delete";
    private static final String RENAME_MAILBOX_URL = "/mailbox/{userId}/rename/{oldUserId}";
    private static final String MESSAGE_READ_URL = "/mailbox/{userId}/message/{messageId}/heard";
    private static final String MOVE_MSG = "/mailbox/{userId}/"
            + "message/{messageId}/move/{destination}";
    private static final String DEL_MSG_URL = "/mailbox/{userId}/message/{messageId}/delete";
    private static final String SAVE_SUBJECT = "/mailbox/{userId}/message/{messageId}/subject";
    private static final String RESTORE_LOG_URL = "/manage/restore/log";
    private static final Log LOG = LogFactory.getLog(RemoteMailboxManagerImpl.class);
    private XPathOperations m_xPathTemplate;
    private RestTemplate m_restTemplate;
    private ConfigCommands m_configCommands;
    private String m_backupDirectory;

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
        try {
            Address ivrRestAddress = getAddressManager().getSingleAddress(Ivr.REST_API);
            Source voicemails = m_restTemplate.getForObject(ivrRestAddress + url, Source.class, urlVariables);
            List<Voicemail> voicemailList = m_xPathTemplate.evaluate("//message", voicemails,
                    new NodeMapper<Voicemail>() {
                        @Override
                        public Voicemail mapNode(Node node, int pos) {
                            return new RemoteVoicemail((Element) node, (String) urlVariables[0],
                                    (String) urlVariables[1]);
                        }
                    });
            return voicemailList;
        } catch (RestClientException ex) {
            throw createUserException(ex.getMessage());
        }
    }

    @Override
    public void deleteMailbox(String userId) {
        Address ivrRestAddress = getAddressManager().getSingleAddress(Ivr.REST_API);
        put(ivrRestAddress + DELETE_MAILBOX_URL, null, userId);
    }

    @Override
    public void renameMailbox(String oldUserId, String newUserId) {
        Address ivrRestAddress = getAddressManager().getSingleAddress(Ivr.REST_API);
        put(ivrRestAddress + RENAME_MAILBOX_URL, null, newUserId, oldUserId);
    }

    @Override
    public void markRead(String userId, String messageId) {
        Address ivrRestAddress = getAddressManager().getSingleAddress(Ivr.REST_API);
        put(ivrRestAddress + MESSAGE_READ_URL, null, userId, messageId);
    }

    @Override
    public void move(String userId, Voicemail voicemail, String destinationFolderId) {
        Address ivrRestAddress = getAddressManager().getSingleAddress(Ivr.REST_API);
        put(ivrRestAddress + MOVE_MSG, null, userId, voicemail.getMessageId(),
                destinationFolderId);
    }

    @Override
    public void delete(String userId, Voicemail voicemail) {
        Address ivrRestAddress = getAddressManager().getSingleAddress(Ivr.REST_API);
        put(ivrRestAddress + DEL_MSG_URL, null, userId, voicemail.getMessageId());
    }

    @Override
    public void save(Voicemail voicemail) {
        Address ivrRestAddress = getAddressManager().getSingleAddress(Ivr.REST_API);
        put(ivrRestAddress + SAVE_SUBJECT, voicemail.getSubject(),
                voicemail.getUserId(), voicemail.getMessageId());
    }

    @Override
    public boolean performBackup(File workingDir) {
        List<Location> locations = getFeatureManager().getLocationsForEnabledFeature(Ivr.FEATURE);
        if (locations != null && locations.size() > 0) {
            Location ivrLocation = locations.get(0);
            try {
                m_configCommands.collectVmBackup(ivrLocation);
                File destfile = new File(workingDir, BackupPlan.VOICEMAIL_ARCHIVE);
                if (destfile.exists()) {
                    FileUtils.deleteQuietly(destfile);
                }
                if (!ivrLocation.isPrimary()) {
                    m_configCommands.uploadVmBackup(getLocationsManager().getPrimaryLocation());
                    FileUtils.moveFile(
                            new File(m_backupDirectory, "voicemail-" + ivrLocation.getFqdn() + ".tar.gz"), destfile);
                } else {
                    FileUtils.moveFile(new File(m_backupDirectory, BackupPlan.VOICEMAIL_ARCHIVE), destfile);
                }
            } catch (IOException ex) {
                LOG.error(String.format("Failed to retrieve backup from voicemail server %s", ex.getMessage()));
                return false;
            }
        }
        return true;
    }

    @Override
    public void performRestore(BackupBean archive, boolean noRestart) {
        // upload archive and restore on ivr side
        List<Location> locations = getFeatureManager().getLocationsForEnabledFeature(Ivr.FEATURE);
        if (locations != null && locations.size() > 0) {
            Location ivrLocation = locations.get(0);
            Writer wtr = null;
            try {
                File f = new File(((ConfigManager) m_configCommands).getGlobalDataDirectory(), "backup.ini");
                wtr = new FileWriter(f);
                wtr.write(archive.getPath());
                wtr.flush();
                m_configCommands.restoreVmBackup(ivrLocation);
            } catch (IOException ex) {
                LOG.error(String.format("Failed to restore backup on voicemail server %s", ex.getMessage()));
                throw new UserException("&error.ivrrestore.failed");
            } finally {
                IOUtils.closeQuietly(wtr);
            }
        }
    }

    @Override
    public String getMailboxRestoreLog() {
        Address ivrRestAddress = getAddressManager().getSingleAddress(Ivr.REST_API);
        StringBuilder log = new StringBuilder();
        log.append(String.format("\n\n:::: Voicemail restore log on remote server %s ::::\n\n",
                ivrRestAddress.getAddress()));
        log.append(m_restTemplate.getForObject(ivrRestAddress + RESTORE_LOG_URL, String.class));
        return log.toString();
    }

    public void setXpathTemplate(XPathOperations xpathTemplate) {
        m_xPathTemplate = xpathTemplate;
    }

    public void setBackupDirectory(String dir) {
        m_backupDirectory = dir;
    }

    @Required
    public void setConfigCommands(ConfigCommands configCommands) {
        m_configCommands = configCommands;
    }

    public void setRestTemplate(RestTemplate restTemplate) {
        HostnameVerifier verifier = new NullHostnameVerifier();
        VmSimpleClientHttpRequestFactory factory = new VmSimpleClientHttpRequestFactory(verifier);
        restTemplate.setRequestFactory(factory);
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
        Address ivrAddress = getAddressManager().getSingleAddress(Ivr.REST_API);
        return new UserException("&error.ivr.server", ivrAddress.getAddress(), message);
    }

    private static class NullHostnameVerifier implements HostnameVerifier {
        public boolean verify(String hostname, SSLSession session) {
            return true;
        }
    }

    private static class VmSimpleClientHttpRequestFactory extends SimpleClientHttpRequestFactory {

        private final HostnameVerifier m_verifier;

        public VmSimpleClientHttpRequestFactory(HostnameVerifier verifier) {
            m_verifier = verifier;
        }

        @Override
        protected void prepareConnection(HttpURLConnection connection, String httpMethod) throws IOException {
            if (connection instanceof HttpsURLConnection) {
                ((HttpsURLConnection) connection).setHostnameVerifier(m_verifier);
            }
            super.prepareConnection(connection, httpMethod);
        }

    }

}
