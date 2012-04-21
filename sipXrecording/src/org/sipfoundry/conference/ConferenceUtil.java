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
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Properties;

import org.apache.commons.httpclient.Credentials;
import org.apache.commons.httpclient.HttpClient;
import org.apache.commons.httpclient.HttpStatus;
import org.apache.commons.httpclient.UsernamePasswordCredentials;
import org.apache.commons.httpclient.auth.AuthPolicy;
import org.apache.commons.httpclient.auth.AuthScope;
import org.apache.commons.httpclient.methods.GetMethod;
import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.util.DomainConfiguration;

public class ConferenceUtil {
    private static String sourceName = "/tmp/freeswitch/recordings";
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxrecording");

    public static void saveInMailboxSynch(String confName) {
        Properties props = getConfProps(confName);
        String ownerName = props.getProperty("ownerName");
        String ownerId = props.getProperty("ownerId");
        String bridgeContact = props.getProperty("bridgeContact");
        String mboxServer = props.getProperty("mboxServer");
        HttpClient httpClient = new HttpClient();              
        
        String urlString = "http://" + mboxServer + "/recording/conference" +
                        "?wn=" + getWavName(confName) +
                        "&on=" + ownerName +
                        "&oi=" + ownerId +
                        "&bc=" + bridgeContact +
                        "&synchronous=true";
        GetMethod triggerRecording = new GetMethod(urlString);
        try {
            int statusCode = httpClient.executeMethod(triggerRecording);
            if (statusCode != HttpStatus.SC_OK) {
                LOG.error("Save recording::failure "+triggerRecording.getStatusLine());
            }
            InputStream stream = triggerRecording.getResponseBodyAsStream();
            LOG.info(IOUtils.toString(stream));
            stream.close();
        } catch (IOException e) {
            LOG.error("Save recording::Trigger error ", e);
        } finally {
            triggerRecording.releaseConnection();
            FileUtils.deleteQuietly(getPropertiesFile(confName));
            FileUtils.deleteQuietly(new File(getWavPath(confName)));
        }
    }

    public static void createSourceDir() {
        File dir = new File(sourceName);
        if (!dir.exists()) {
           dir.mkdirs();
        }
    }

    public static void saveConfProperties(String confName, String ownerName, String ownerId, String bridgeContact, String mboxServer) {
        Properties props = new Properties();
        props.put("ownerName", ownerName);
        props.put("ownerId", ownerId);
        props.put("bridgeContact", bridgeContact);
        props.put("mboxServer", mboxServer);
        OutputStream out = null;
        try {
            out = new FileOutputStream(getPropertiesFile(confName));
            props.store(out, "conference properties");
        } catch (Exception ex) {
            LOG.error("Cannot save conference properties", ex);
        } finally {
            IOUtils.closeQuietly(out);
        }
    }

    private static File getPropertiesFile(String confName) {
        File f = new File(sourceName, confName+".properties");
        if (!f.exists()) {
            try {
                f.createNewFile();
            } catch (IOException ex) {
                LOG.error("Cannot retrieve conference properties file", ex);
            }
        }
        return f;
    }

    private static Properties getConfProps(String confName) {
        Properties props = new Properties();
        InputStream is = null;
        try {
            is = new FileInputStream(getPropertiesFile(confName));
            props.load(is);
        } catch(Exception ex) {
            LOG.error("Cannot load properties file", ex);
        } finally {
            IOUtils.closeQuietly(is);
        }
        return props;
    }

    public static String getWavName(String confName) {
        return confName + ".wav";
    }

    public static String getWavPath(String confName) {
        return sourceName + File.separator + getWavName(confName);
    }

    public static boolean existsProps(String confName) {
        return new File(sourceName, confName+".properties").exists();
    }

    public static boolean existsWav(String confName) {
        return new File(sourceName, getWavName(confName)).exists();
    }

    public static boolean isRecordingInProgress(String confName) {
        return existsWav(confName) && existsProps(confName);
    }
}
