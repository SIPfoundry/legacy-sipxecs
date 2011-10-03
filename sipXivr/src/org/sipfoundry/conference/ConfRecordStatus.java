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
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;
import java.net.URLConnection;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.log4j.Logger;

import org.sipfoundry.voicemail.mailbox.MessageDescriptor;
import org.sipfoundry.voicemail.mailbox.MessageDescriptorWriter;
import org.sipfoundry.voicemail.mailbox.MessageDescriptor.Priority;

/**
 * Trigger the transfer of a conference recording using a simple HTTP request.
 */
public class ConfRecordStatus extends HttpServlet {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    public static final String MessageSummaryContentType = "application/simple-message-summary";

    public static String formatConfRecord(String recording_name) {
        return String.format("Conference-Recording: %s", recording_name);
    }

    /**
     * Generate the next message Id
     * static synchronized as it's machine wide
     * @param directory which holds the messageid-conference.txt file
     */
    static synchronized String nextMessageId(String directory) {
        long numericMessageId = 1;
        String format = "%08d";
        String messageId = String.format(format,numericMessageId);

        // messageid-conference.txt file is (hopefully) in the directory
        File midFile = new File(directory, "messageid-conference.txt");
        String messageIdFilePath = midFile.getPath();
        if (midFile.exists()) {
            try {
                // The messageid in the file is the NEXT one
                messageId = FileUtils.readFileToString(midFile);
                numericMessageId = Long.parseLong(messageId);
            } catch (IOException e) {
                LOG.error("Message::nextMessageId cannot read "+messageIdFilePath, e);
                throw new RuntimeException(e);
            }
        }
        // Increment message id, store for another day
        numericMessageId++;
        try {
            FileUtils.writeStringToFile(midFile, String.format(format, numericMessageId));
        } catch (IOException e) {
            LOG.error("Message::nextMessageId cannot write "+messageIdFilePath, e);
            throw new RuntimeException(e);
        }

        return messageId;
    }

    @Override
    public void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        String inString = request.getQueryString();

        LOG.info(String.format("ConfRecordStatus::doGet Conference Record string: " + inString ));

        // The get command contains the required information
        String parmWavName = request.getParameter("wn");
        String parmOwnerName = request.getParameter("on");
        String parmOwnerId = request.getParameter("oi");
        String parmBridgeContact = request.getParameter("bc");
        String synch = request.getParameter("synchronous");
        Boolean synchronous = (synch == null) ? false : new Boolean(synch);

        boolean stringsOK = ((parmWavName!=null) && (parmOwnerName!=null) &&
                             (parmOwnerId!=null) && (parmBridgeContact!=null) &&
                             (parmWavName.compareTo("") != 0) &&
                             (parmOwnerName.compareTo("") != 0) &&
                             (parmOwnerId.compareTo("") != 0) &&
                             (parmBridgeContact.compareTo("") != 0));

        response.setContentType(MessageSummaryContentType);
        // Use the OutputStream rather than the PrintWriter as this will cause Jetty
        // To NOT set the charset= parameter on the content type, which breaks
        // The status server doing this request.
        // The price is we aren't specifying the encoding of this message (sigh).
        OutputStream os = response.getOutputStream();

        // Just echo the bytes of the string.  No character encoding or nothing.
        os.write(formatConfRecord(parmWavName).getBytes());
        if (!synchronous) {
            os.close();
        }

        // The WAV file is now the remote conference server
        // Stream the file to the local voicemail server
        if (stringsOK) {
            try {
                String confMailboxName = parmOwnerName;

                // If the conference directory doesn't exist then create it now
                // Should be based on SIPX_DATADIR "/var/sipxdata/mediaserver/data");
                String mediaDataDirectory = System.getProperty("var.dir") + "/mediaserver/data";
                String baseDirectory = mediaDataDirectory + "/mailstore/" +
                                       confMailboxName + "/conference";
                File baseDirFile = new File(baseDirectory);
                if (!baseDirFile.exists()) {
                    baseDirFile.mkdirs();
                }

                // Get the next message sequence number.
                String m_messageId = nextMessageId(mediaDataDirectory);
                // Conference recordings use a file name of "1" plus the 8-digit ID to differentiate
                // them from the regular 8-digit voicemail message file name.
                String baseName = baseDirectory + "/" +"1"+m_messageId+"-00";
                File audioFile = new File(baseName+".wav");

                // Get the name of the server that sent us the request
                String remoteHost = request.getRemoteHost();

                URL url  = new URL("http://" + remoteHost + ":8090/recordings/" + parmWavName);
                URLConnection urlC = url.openConnection();
                InputStream streamIn = urlC.getInputStream();
                OutputStream streamOut = new FileOutputStream(audioFile);
                IOUtils.copy(streamIn, streamOut);
                IOUtils.closeQuietly(streamIn);
                IOUtils.closeQuietly(streamOut);

                // Get the WAV file duration in seconds, ignore files that are
                // so short they round off to 0 seconds.
                long wavDuration = audioFile.length()/(8000*2);
                if (wavDuration > 0) {
                    MessageDescriptor messageDescriptor = new MessageDescriptor();
                    messageDescriptor.setId(parmOwnerId);
                    messageDescriptor.setFromUri(parmBridgeContact);
                    messageDescriptor.setDurationSecs(wavDuration);
                    messageDescriptor.setTimestamp(audioFile.lastModified());
                    messageDescriptor.setSubject("Conference Call "+m_messageId);
                    messageDescriptor.setPriority(Priority.NORMAL);

                    File descriptorFile = new File(baseName+".xml");
                    new MessageDescriptorWriter().writeObject(messageDescriptor, descriptorFile);
                }
                else {
                    LOG.info(String.format("ConfRecordStatus::short WAV file received %d", audioFile.length()));
                }
            } catch (IOException e) {
                LOG.error("ConfRecordStatus::Copy IO error ", e);
            } finally {
                IOUtils.closeQuietly(os);
            }
        }
    }
}
