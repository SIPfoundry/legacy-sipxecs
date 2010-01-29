/*
 *
 *
 * Copyright (C) 2008-2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.freeswitch;

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

    public FreeSwitchEventSocket(FreeSwitchConfigurationInterface config) {
        super(config);
        this.setConfig(config);
    }

    /**
     * Given the socket from a FreeSwitch "outbound" call, do the "connect" dance to collect all
     * the variables FreeSwitch reports.
     *
     * Enable FreeSwitch to report async events of interest.
     */
    public boolean connect(Socket socket) throws IOException {
        return connect(socket, null);
    }
    
    /**
     * Given the socket from a FreeSwitch "outbound" call, do the "connect" dance to collect all
     * the variables FreeSwitch reports.
     * 
     * Enable FreeSwitch to report async events of interest.
     */
    public boolean connect(Socket socket, String authPassword) throws IOException {
        m_clientSocket = socket;
        m_out = new PrintWriter(m_clientSocket.getOutputStream(), true);
        setIn(new BufferedReader(new InputStreamReader(m_clientSocket.getInputStream())));

        if (authPassword != null) {
            cmdResponse("auth " + authPassword);
        }

        // Accept the connection from FreeSwitch, and get the variables for this call
        FreeSwitchEvent event = cmdResponse("connect");
        if (event.isEmpty()) {
        	return false;
        }

        LOG.debug(event.getResponse());
        setVariables(FreeSwitchEvent.parseHeaders(event.getResponse()));      
        
        String UUID = getVariable("caller-unique-id");
        setSessionUUID(UUID);
        
        // Enable reporting of interesting events
         
        if(UUID != null) {
            cmdResponse("event plain all");
            cmdResponse("filter Unique-ID " + UUID);
        } else {
            cmdResponse("myevents");   
        }
         
        return true;
    }

    /**
     * Send a command to FreeSwitch.
     */
    public void cmd(String cmd) {
        LOG.debug("FSES::cmd " + cmd);
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
        LOG.debug("FSES::cmdResponse " + cmd);
        // Send the command
        m_out.printf("%s%n%n", cmd);

        // Read events off the socket until the command/reply is seen.
        // Other events are queued onto the eventQueue
        for (;;) {
            FreeSwitchEvent event = awaitLiveEvent();

            if (event.isEmpty()) {
            	// Hey! FS closed the socket on us!
            	return event;
            } else if (event.getContentType().contentEquals("command/reply")) {
                LOG.debug(String.format("FSES::cmdResponse cmd (%s) response (%s)", cmd, event.getHeader("Reply-Text", "(No Reply-Text)")));
                return event;
            } else {
                // Push non reply event onto the queue for someone else to deal with.
                getEventQueue().add(event);
            }
        }
    }

    /**
     * Send an api command to FreeSwitch and await the response.
     *
     * Any events sent before the response arrives are queued on the eventQueue.
     *
     */
    public FreeSwitchEvent apiCmdResponse(String cmd) {
        LOG.debug("FSES::apiCmdResponse " + cmd);
        // Send the command
        m_out.printf("api %s%n%n", cmd);

        // Read events off the socket until the api/response is seen.
        // Other events are queued onto the eventQueue
        for (;;) {
            FreeSwitchEvent event = awaitLiveEvent();

            if (event.isEmpty()) {
            	// Hey! FS closed the socket on us!
            	return event;
            } else if (event.getContentType().contentEquals("api/response")) {
                LOG.debug(String.format("FSES::apiCmdResponse cmd (%s) response (%s)", cmd, event.getHeader(
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
 //               LOG.debug("::awaitLiveEvent event: " + line);
                if (line.startsWith("Content-Length:")) {
                    contentLength = line;
                }
            }
        } catch (Exception e) {
            LOG.error("FSES::awaitLiveEvent readLine exception", e);
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
                    LOG.error("FSES::awaitEvent content exception", e);
                    return new FreeSwitchEvent(response, null, e);
                }
            }
        }
        event = new FreeSwitchEvent(response, content);
        LOG.debug(String.format("FSES::awaitEvent live response (%s) Event-Name (%s) Application (%s)",
                event.getContentType(), event.getEventValue("Event-Name", "(null)"), event.getEventValue("Application", "(null)")));
        
        // Look for a "uuid_bridge" operation which indicates that this FS session
        // is a consultative transfer target.
        if (event.getContentType().contentEquals("text/event-plain")
                && event.getEventValue("Event-Name", "(null)").contentEquals("CHANNEL_EXECUTE")
                && event.getEventValue("Application", "(null)").contentEquals("uuid_bridge")) {
            // Retrieve new UUID and update filters.
            String newUUID = event.getEventValue("Application-Data");
            setSessionUUID(newUUID);
            cmd("filter Unique-ID " + newUUID);
            LOG.debug(String.format("FSES::awaitEvent uuid_bridge to: %s", newUUID));
        }

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
