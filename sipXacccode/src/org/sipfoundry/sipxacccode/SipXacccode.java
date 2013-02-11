/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxacccode;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Hashtable;
import java.util.Properties;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;
import org.sipfoundry.authcode.AuthCode;
import org.sipfoundry.commons.freeswitch.Answer;
import org.sipfoundry.commons.freeswitch.DisconnectException;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocket;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocketInterface;
import org.sipfoundry.commons.freeswitch.Hangup;
import org.sipfoundry.commons.log4j.SipFoundryLayout;

public class SipXacccode implements Runnable {
    private static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxacccode");
    private static AccCodeConfiguration s_config;

    private final Socket m_clientSocket;
    private FreeSwitchEventSocketInterface m_fses;

    public SipXacccode(Socket client) {
        m_clientSocket = client;
    }

    /*
     * header looks like:
     * variable_sip_h_diversion=<tel:3948809>;reason=no-answer;counter=1;screen=no;privacy=off
     */
    private void parseHeader(Hashtable<String, String> parameters) {

        String divHeader = m_fses.getVariable("variable_sip_h_diversion");

        if(divHeader != null) {
            divHeader = divHeader.toLowerCase();
            String[] subParms = divHeader.split(";");

            int index = divHeader.indexOf("tel:");

            if(index >= 0) {
                divHeader = divHeader.substring(index+4);
                index = divHeader.indexOf(">");
                if(index > 0) {
                    divHeader = divHeader.substring(0, index);

                    parameters.put("action", "deposit");
                    parameters.put("origCalledNumber", divHeader);

                    // now look for call forward reason
                    for (String param : subParms) {
                        if(param.startsWith("reason=")) {
                            param = param.substring("reason=".length());
                            param.trim();
                            parameters.put("call-forward-reason", param);
                            break;
                        }
                    }
                }
            }
        }
    }

    /**
     * Determine what to do based on the SIP request.
     *
     */
    @Override
	public void run() {
        LOG.debug("SipXacccode::run Starting SipXacccode thread with client " + m_clientSocket);

        try {
            m_fses = new FreeSwitchEventSocket(s_config);
            if (m_fses.connect(m_clientSocket, null)) {


                        LOG.debug("SipXacccode socket connection, event received");
		        String sipReqParams = m_fses.getVariable("variable_sip_req_params");
		        // Create a table of parameters to pass in
		        Hashtable<String, String> parameters = new Hashtable<String, String>();

		        if (sipReqParams != null) {
		            // Split parameter fields (separated by semicolons)
		            String[] params = sipReqParams.split(";");
		            for (String param : params) {
		                // Split key value pairs (separated by optional equal sign)
		                String[] kvs = param.split("=", 2);
		                if (kvs.length == 2) {
		                    parameters.put(kvs[0], kvs[1]);
		                } else {
		                    parameters.put(kvs[0], "");
		                }
		            }
		        }

		        LOG.info(String.format("SipXacccode::run Accepting call-id %s from %s to %s",
		                m_fses.getVariable("variable_sip_call_id"),
		                m_fses.getVariable("variable_sip_from_uri"),
		                m_fses.getVariable("variable_sip_req_uri")));

                        // Answer the call.
		        m_fses.invoke(new Answer(m_fses));

		        String action = parameters.get("command");
		        if (action == null) {
		            LOG.warn("Cannot determine which application to run as the action parameter is missing.");
		        } else if (action.contentEquals("auth")) {
		            // Run the Authorization Code app.
		            AuthCode app = new AuthCode(s_config, m_fses, parameters);
		            app.run();
		        } else if (action.equals("disa")) {
		                LOG.warn("Should start disa app");
		                parseHeader(parameters);
                        } else if (action.equals("acct")) {
		            // Run the Account Code app.
		            //AcctCode app = new AcctCode(s_config, m_fses, parameters);
		            //app.run();
		        } else {
		            // Nothing else to run...
		            LOG.warn("Cannot determine which application to run from command="+ action);
		        }

		        m_fses.invoke(new Hangup(m_fses));
            }
        } catch (DisconnectException e) {
            LOG.info("SipXacccode::run Far end hungup.");
        } catch (Throwable t) {
            LOG.error("SipXacccode::run", t);
        } finally {
            try {
                m_fses.close();
            } catch (IOException e) {
                // Nothing to do, no where to go home...
            }
        }

        LOG.debug("SipXacccode::run Ending SipXacccode thread with client " + m_clientSocket);
    }


    /**
     * Load the configuration from the sipxacccode.properties file.
     * Wait for FreeSWITCH to make a TCP connection to s_eventSocketPort.
     * Spawn off a thread to handle each connection.
     *
     * @throws Throwable
     */
    static void init() throws Throwable {
        int eventSocketPort;

        // Load the configuration
        s_config = AccCodeConfiguration.get();

        // Configure log4j
        Properties props = new Properties();
        props.setProperty("log4j.rootLogger", "warn, file");
        props.setProperty("log4j.logger.org.sipfoundry.sipxacccode", SipFoundryLayout
                .mapSipFoundry2log4j(s_config.getLogLevel()).toString());
        props.setProperty("log4j.appender.file", "org.sipfoundry.commons.log4j.SipFoundryAppender");
        props.setProperty("log4j.appender.file.File", s_config.getLogFile());
        props.setProperty("log4j.appender.file.layout", "org.sipfoundry.commons.log4j.SipFoundryLayout");
        props.setProperty("log4j.appender.file.layout.facility", "sipXacccode");
        PropertyConfigurator.configure(props);

        eventSocketPort = s_config.getEventSocketPort();
        LOG.info("Starting SipXacccode listening on port " + eventSocketPort);
        ServerSocket serverSocket = new ServerSocket(eventSocketPort);
        for (;;) {
            Socket client = serverSocket.accept();
            SipXacccode acccode = new SipXacccode(client);
            Thread thread = new Thread(acccode);
            thread.start();
        }
    }

    /**
     * Main entry point for sipXacccode
     * @param args
     */
    public static void main(String[] args) {
        try {
            init();
        } catch (Exception e) {
            LOG.fatal(e,e);
            e.printStackTrace();
            System.exit(1);
        } catch (Throwable t) {
            LOG.fatal(t,t);
            t.printStackTrace();
            System.exit(1);
        }
    }
}
