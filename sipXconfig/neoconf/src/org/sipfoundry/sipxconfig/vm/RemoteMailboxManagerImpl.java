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

import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;
import java.util.List;

import javax.xml.transform.Source;

import org.apache.commons.httpclient.HttpClient;
import org.apache.commons.httpclient.methods.PutMethod;
import org.apache.commons.httpclient.methods.multipart.FilePart;
import org.apache.commons.httpclient.methods.multipart.MultipartRequestEntity;
import org.apache.commons.httpclient.methods.multipart.Part;
import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.BackupBean;
import org.sipfoundry.sipxconfig.admin.Restore;
import org.sipfoundry.sipxconfig.common.UserException;
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
    private static final String RESTORE_LOG_URL = "https://{host}:{port}/manage/restore/log";
    private static final Log LOG = LogFactory.getLog(RemoteMailboxManagerImpl.class);
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
    public boolean performBackup(File workingDir) {
        OutputStream outputStream = null;
        InputStream inputStream = null;
        try {
            URL backupUrl = new URL(getMailboxServerUrl() + "/manage/backup");
            outputStream = FileUtils.openOutputStream(new File(workingDir, "voicemail.tar.gz"));
            inputStream = backupUrl.openStream();
            IOUtils.copy(inputStream, outputStream);
        } catch (Exception ex) {
            LOG.error(String.format("Failed to retrieve backup from voicemail server %s", ex.getMessage()));
            return false;
        } finally {
            IOUtils.closeQuietly(inputStream);
            IOUtils.closeQuietly(outputStream);
        }
        return true;
    }

    @Override
    public void performRestore(BackupBean archive, boolean verify, boolean noRestart) {
        if (verify) {
            // verify locally if valid archive
            Restore.runRestoreScript(getBinDir(), archive, verify, true);
        } else {
            // upload archive and restore on ivr side
            File vmArchive = new File(archive.getPath());
            PutMethod method = new PutMethod(getMailboxServerUrl() + "/manage/restore");
            try {
                Part[] parts = {new FilePart(vmArchive.getName(), vmArchive)};
                method.setRequestEntity(new MultipartRequestEntity(parts, method.getParams()));
                HttpClient client = new HttpClient();
                client.executeMethod(method);
            } catch (Exception ex) {
                LOG.error(String.format("Failed to restore backup on voicemail server %s", ex.getMessage()));
                throw new UserException("&error.ivrrestore.failed");
            } finally {
                method.releaseConnection();
            }
        }
    }

    @Override
    public String getMailboxRestoreLog() {
        StringBuilder log = new StringBuilder();
        log.append(String.format("\n\n:::: Voicemail restore log on remote server %s ::::\n\n", getHost()));
        log.append(m_restTemplate.getForObject(RESTORE_LOG_URL, String.class, getHost(), getPort()));
        return log.toString();
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
