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
package org.sipfoundry.conference;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;

import org.apache.commons.httpclient.HttpClient;
import org.apache.commons.httpclient.HttpStatus;
import org.apache.commons.httpclient.methods.GetMethod;
import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.confdb.Conference;
import org.sipfoundry.commons.confdb.ConferenceService;
import org.sipfoundry.sipxrecording.RecordingConfiguration;

public class ConferenceContextImpl {
    private static String sourceName = "/tmp/freeswitch/recordings";
    private static String lastGoodIvr = null;
    private static ConferenceContextImpl m_conferenceContextImpl;
    private ConferenceService m_conferenceService;
    private RecordingConfiguration m_recordingConfig;
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxrecording");

    private ConferenceContextImpl() {
    }

    public static synchronized ConferenceContextImpl getInstance() {
        if (m_conferenceContextImpl == null) {
            m_conferenceContextImpl = new ConferenceContextImpl();
        }
        return m_conferenceContextImpl;
    }

    public Conference getConference(String confName) {
        return m_conferenceService.getConference(confName);
    }
    public void saveInMailboxSynch(String confName) {
        Conference conf = m_conferenceService.getConference(confName);
        notifyIvr(getIvrUris(), getFileName(confName), conf, true);
        FileUtils.deleteQuietly(new File(getFilePath(confName)));
    }

    public void createSourceDir() {
        File dir = new File(sourceName);
        if (!dir.exists()) {
           dir.mkdirs();
        }
    }

    /**
     * Trigger the servlet on the voicemail server to read the recorded file E.g.
     * "http://s1.example.com:8086/recording/conference?test1_6737347.wav or
     * http://s1.example.com:8086/recording/conference?test1_6737347.mp3"
     */
    private void notifyIvr(String ivrUri, String fileName, Conference conf, boolean synchronous)
            throws IOException {
        String username = conf.getConfOwner();
        HttpClient httpClient = new HttpClient();
        String urlString = ivrUri + "/recording/conference"
            + "?wn=" + fileName
            + "&on=" + conf.getConfOwner()
            + "&bc=" + conf.getUri()
            + "&synchronous=" + synchronous;
        LOG.debug("Notify IVR to pick the recorded file and copy it into user mailbox: " + urlString);
        GetMethod triggerRecording = new GetMethod(urlString);
        try {
            int statusCode = httpClient.executeMethod(triggerRecording);
            if (statusCode != HttpStatus.SC_OK) {
                LOG.error("Save recording::failure "+triggerRecording.getStatusLine());
            }
            InputStream stream = triggerRecording.getResponseBodyAsStream();
            LOG.debug(IOUtils.toString(stream));
            stream.close();
            lastGoodIvr = ivrUri;
        } finally {
            triggerRecording.releaseConnection();
        }
    }

    public void notifyIvr(String[] ivrUris, String fileName, Conference conf, boolean synchronous) {
        if (lastGoodIvr != null) {
            try {
                notifyIvr(lastGoodIvr, fileName, conf, synchronous);
                return;
            } catch (IOException ex) {
                //do not throw exception as we have to iterate through all nodes
                LOG.error("ConfRecordThread::Trigger error on last good ivr node:" + lastGoodIvr);
            }
        }
        for (String ivrUri : ivrUris) {
            //do not contact last Ivr uri again
            if (StringUtils.equals(lastGoodIvr, ivrUri)) {
                continue;
            }
            try {
                notifyIvr(ivrUri, fileName, conf, synchronous);
                return;
            } catch (IOException ex) {
                LOG.error("ConfRecordThread::Trigger error on node:" + ivrUri);
            }
        }
    }

    public String getFileName(String confName) {
        return confName + "." + m_recordingConfig.getAudioFormat();
    }

    public String getFilePath(String confName) {
        return sourceName + File.separator + getFileName(confName);
    }

    public boolean existsFile(String confName) {
        return new File(sourceName, getFileName(confName)).exists();
    }

    public boolean isRecordingInProgress(String confName) {
        return existsFile(confName);
    }

    public void setConferenceService(ConferenceService conferenceService) {
        m_conferenceService = conferenceService;
    }

    public void setRecordingConfig(RecordingConfiguration recordingConfig) {
        m_recordingConfig = recordingConfig;
    }

    public String getSendIMUrl() {
        return m_recordingConfig.getSendImUrl();
    }

    public String[] getIvrUris() {
        return m_recordingConfig.getIvrNodes();
    }

    public String getAudioFormat() {
        return m_recordingConfig.getAudioFormat();
    }

}
