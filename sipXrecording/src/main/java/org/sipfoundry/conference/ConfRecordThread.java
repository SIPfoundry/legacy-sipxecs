/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.conference;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;
import java.net.URLConnection;
import java.net.URL;
import java.util.Collection;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.io.IOUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.ConferenceTask;
import org.sipfoundry.commons.freeswitch.ConfBasicThread;
import org.sipfoundry.commons.freeswitch.FreeSwitchEvent;
import org.sipfoundry.sipxrecording.RecordingConfiguration;
import org.sipfoundry.sipxrecording.RecordCommand;

public class ConfRecordThread extends ConfBasicThread {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxrecording");

    static String sourceName = "/tmp/freeswitch/recordings";
    static String destName = System.getProperty("var.dir") + "/mediaserver/data/recordings";

    public ConfRecordThread(RecordingConfiguration recordingConfig) {
        // Check that the freeswitch initial recording directory exists
        File sourceDir = new File(sourceName);
        if (!sourceDir.exists()) {
           sourceDir.mkdirs();
        }

        // Create the distribution directory if it doesn't already exist.
        File destDir = new File(destName);
        if (!destDir.exists()) {
            try {
                Process process = Runtime.getRuntime().exec(new String[] {"ln", "-s", sourceName, destName});
                process.waitFor();
                process.destroy();
            } catch (IOException e) {
                LOG.error("ConfRecordThread::IOException error ", e);
            } catch (InterruptedException e) {
                LOG.error("ConfRecordThread::InterruptedException error ", e);
            }
        }

        setConfConfiguration(recordingConfig);
    }

    private void AuditWavFiles(File dir) {
        // If any WAV file on the directory is more than 5 minutes old then delete it.
        File[] children = dir.listFiles();
        if (children != null) {
            for (int i=0; i<children.length; i++) {
                File child = children[i];
                String filename = child.getName();
                if (filename.endsWith(".wav")) {
                   if ((System.currentTimeMillis() - child.lastModified())/1000 > 5*60) {
                       child.delete();
                       LOG.debug("ConfRecordThread::Audit deleting " + filename);
                   }
                }
            }
        }
    }

    public void ProcessConfStart(FreeSwitchEvent event, ConferenceTask conf) {
        String confName = event.getEventValue("conference-name");
        ConferenceBridgeItem item = ConferenceBridgeXML.getConferenceBridgeItem(confName);
        if (item != null) {
            String uniqueId = event.getEventValue("Unique-ID");
            String wavName = new String(confName + "_" + uniqueId + ".wav");
            LOG.debug("ConfRecordThread::Creating conference recording " + wavName);
            String confCmd = "record " + sourceName + "/" + wavName;
            // Send the start recording command to Freeswitch
            RecordCommand recordcmd = new RecordCommand(getCmdSocket(), confName, confCmd);
            recordcmd.go();
            // Store the file name
            conf.setWavName(wavName);
        }
    }

    public void ProcessConfEnd(FreeSwitchEvent event, ConferenceTask conf) {
        String confName = event.getEventValue("conference-name");
        ConferenceBridgeItem item = ConferenceBridgeXML.getConferenceBridgeItem(confName);
        if (item != null) {
            String wavName = conf.getWavName();
            LOG.debug("ConfRecordThread::Finished conference recording of " + wavName);

            try {
                // Let FS finish any recording it may be doing
                Thread.sleep(1000);
            } catch (InterruptedException e) {
            }

            AuditWavFiles(new File(destName));

            // Use the conference name to find the conference mailbox server.
            String mboxServerAndPort = item.getMailboxServer();

            // Trigger the servlet on the voicemail server to read the WAV file
            // E.g. "http://s1.example.com:8085/recording/conference?test1_6737347.wav"
            try {
                String urlString = "http://" + mboxServerAndPort +
                                   "/recording/conference" + 
                                   "?wn=" + wavName + 
                                   "&on=" + item.getOwnerName() +
                                   "&oi=" + item.getOwnerId() +
                                   "&bc=" + item.getBridgeContact();
                URL url  = new URL(urlString);
                URLConnection urlC = url.openConnection();
                long contentLength = urlC.getContentLength();
            } catch (IOException e) {
                LOG.error("ConfRecordThread::Trigger error ", e);
            }
        }
    }
}
