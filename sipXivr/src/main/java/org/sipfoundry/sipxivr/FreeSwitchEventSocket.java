/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxivr;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.Vector;

/**
 * The nitty-gritty handling of data to and from the socket that came from a FreeSwitch "outbound"
 * call.
 * 
 * Deals with marshalling the data into FreeSwitchEvents, and blocks waiting for commands to
 * finish.
 * 
 */
public class FreeSwitchEventSocket extends FreeSwitchEventSocketInterface {
    private Socket m_clientSocket;
    private PrintWriter m_out;
    private BufferedReader m_in;

    public FreeSwitchEventSocket(Configuration config) {
        super();
        this.setConfig(config);
    }

    /**
     * Given the socket from a FreeSwitch "outbound" call, do the "connect" dance to collect all
     * the variables FreeSwitch reports.
     * 
     * Enable FreeSwitch to report async events of interest.
     */
    public void connect(Socket socket) throws IOException {
        m_clientSocket = socket;
        m_out = new PrintWriter(m_clientSocket.getOutputStream(), true);
        setIn(new BufferedReader(new InputStreamReader(m_clientSocket.getInputStream())));

        // Accept the connection from FreeSwitch, and get the variables for this call
        FreeSwitchEvent event = cmdResponse("connect");
        setVariables(FreeSwitchEvent.parseHeaders(event.getResponse()));

        // Enable reporting of interesting events
        cmdResponse("myevents");
    }

    /**
     * Send a command to FreeSwitch.
     */
    public void cmd(String cmd) {
        LOG.debug("::cmd " + cmd);
        // Send the command
        m_out.printf("%s%n%n", cmd);
    }

    /**
     * Send a command to FreeSwitch and await the response.
     * 
     * Any events sent before the response arrives are queued on the eventQueue.
     * 
     */
    public FreeSwitchEvent cmdResponse(String cmd) {
        // Send the command
        m_out.printf("%s%n%n", cmd);

        // Read events off the socket until the command/reply is seen.
        // Other events are queued onto the eventQueue
        for (;;) {
            FreeSwitchEvent event = awaitLiveEvent();

            if (event.getContentType().contentEquals("command/reply")) {
                LOG.debug(String.format("cmd (%s) response (%s)", cmd, event.getHeader(
                        "Reply-Text", "(No Reply-Text)")));
                return event;
            } else {
                // Push non reply event onto the queue for someone else to deal with.
                getEventQueue().add(event);
            }
        }
    }

    /**
     * Block waiting for an event to arrive on the socket.
     * 
     */
    public FreeSwitchEvent awaitLiveEvent() {
        FreeSwitchEvent event = null;
        Vector<String> response = new Vector<String>();
        String content = null;

        String contentLength = null;

        try {
            for (;;) {
                String line = getIn().readLine();
                if (line == null || line.equals("")) {
                    break;
                }
                response.add(line);
                LOG.debug("::awaitLiveEvent event: " + line);
                if (line.startsWith("Content-Length:")) {
                    contentLength = line;
                }
            }
        } catch (Exception e) {
            LOG.error("::awaitLiveEvent readLine exception", e);
            return new FreeSwitchEvent(response, null, e);
        }

        if (contentLength != null) {
            int length = Integer.parseInt(contentLength.substring(16));
            if (length > 0) {
                try {
                    char[] cbuf = new char[length];
                    int actual = getIn().read(cbuf, 0, length);
                    if (actual >= 0) {
                        content = new String(cbuf);
                    }
                } catch (Exception e) {
                    LOG.error("::awaitEvent content exception", e);
                    return new FreeSwitchEvent(response, null, e);
                }
            }
        }
        event = new FreeSwitchEvent(response, content);
        LOG.debug(String.format("::awaitEvent live response (%s) Event-Name (%s)", event
                .getContentType(), event.getEventValue("Event-Name", "(null)")));
        return event;
    }

    /**
     * Close the connection to FreeSwitch (ends the call)
     * 
     */
    public void close() throws IOException {
        m_clientSocket.close();
    }

    public void setIn(BufferedReader in) {
        m_in = in;
    }

    public BufferedReader getIn() {
        return m_in;
    }
}
