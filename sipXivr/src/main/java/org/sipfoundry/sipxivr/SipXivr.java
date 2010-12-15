/*
 *
 *
 * Copyright (C) 2008-2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxivr;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Hashtable;
import java.util.Properties;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;
import org.sipfoundry.attendant.Attendant;
import org.sipfoundry.commons.freeswitch.Answer;
import org.sipfoundry.commons.freeswitch.ConfBasicThread;
import org.sipfoundry.commons.freeswitch.DisconnectException;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocket;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocketInterface;
import org.sipfoundry.commons.freeswitch.Hangup;
import org.sipfoundry.commons.freeswitch.Set;
import org.sipfoundry.commons.log4j.SipFoundryLayout;
import org.sipfoundry.commons.userdb.ValidUsersXML;
import org.sipfoundry.conference.ConfRecordStatus;
import org.sipfoundry.moh.Moh;
import org.sipfoundry.faxrx.FaxRx;
import org.sipfoundry.voicemail.ConferenceServlet;
import org.sipfoundry.voicemail.Emailer;
import org.sipfoundry.voicemail.ExtMailStore;
import org.sipfoundry.voicemail.MailboxServlet;
import org.sipfoundry.voicemail.Mwistatus;
import org.sipfoundry.voicemail.VoiceMail;
import org.sipfoundry.bridge.Bridge;

public class SipXivr implements Runnable {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private static IvrConfiguration s_config;

    private Socket m_clientSocket;
    private static FreeSwitchEventSocketInterface m_fses;

    public SipXivr(Socket client) {
        m_clientSocket = client;
    }

    /*
     * diversion header looks like:
     * variable_sip_h_diversion=<tel:3948809>;reason=no-answer;counter=1;screen=no;privacy=off
     */
    private void parseDiversionHeader(Hashtable<String, String> parameters) {

        String divHeader = m_fses.getVariable("variable_sip_h_diversion");

        if(divHeader != null) {
            LOG.debug("SipXivr::parseDiversionHeader header=" + divHeader);
            divHeader = divHeader.toLowerCase();
            String[] subParms = divHeader.split(";");
            String ocn = null;

            // Look for the OCN format <tel:3948809>
            if (ocn == null) {
                int index = divHeader.indexOf("<tel:");
                if(index >= 0) {
                    divHeader = divHeader.substring(index+5);
                    index = divHeader.indexOf(">");
                    if(index > 0) {
                        ocn = divHeader.substring(0, index);
                    }
                }
            }

            // Look for the OCN format <sip:3948809@196.8.1.7>
            if (ocn == null) {
                int index = divHeader.indexOf("<sip:");
                if(index >= 0) {
                    divHeader = divHeader.substring(index+5);
                    index = divHeader.indexOf(">");
                    if(index > 0) {
                        String sipOcn = divHeader.substring(0, index);
                        // Ignore the domain for now (it may be an IP address)
                        ocn = ValidUsersXML.getUserPart(sipOcn);
                    }
                }
            }

            if (ocn != null) {
                LOG.debug("SipXivr::parseDiversionHeader OCN=" + ocn);
                parameters.put("action", "deposit");
                parameters.put("origCalledNumber", ocn);

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

    /**
     * Determine what to do based on the SIP request.
     *
     * Run the autoattendant if action=autoattendant
     * Otherwise, hang up.
     */
    public void run() {
        LOG.debug("SipXivr::run Starting SipXivr thread with client " + m_clientSocket);

        try {
            m_fses = new FreeSwitchEventSocket(s_config);
            if (m_fses.connect(m_clientSocket, null)) {


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

		        LOG.info(String.format("SipXivr::run Accepting call-id %s from %s to %s",
		                m_fses.getVariable("variable_sip_call_id"),
		                m_fses.getVariable("variable_sip_from_uri"),
		                m_fses.getVariable("variable_sip_req_uri")));



		        String action = parameters.get("action");
                String uuid = parameters.get("uuid");
                if (uuid != null) {
                    m_fses.invoke(new Answer(m_fses));
                    if (action == null) {
                        LOG.warn("Cannot determine which application to run as the action parameter is missing.");
                    } else if (action.contentEquals("autoattendant")) {
                        // Run the Auto Attendant.
                        Attendant app = new Attendant(s_config, m_fses, parameters);
                        app.run();
                    } else if (action.equals("deposit") || action.equals("retrieve")) {
                            parseDiversionHeader(parameters);
                        // Run VoiceMail
                        VoiceMail app = new VoiceMail(s_config, m_fses, parameters);
                        app.run();
                    } else if (action.equals("faxrx")) {
			// Run fax receive application
			FaxRx app = new FaxRx(s_config, m_fses, parameters);
			app.run() ;
                    } else if (action.equals("moh")) {
                        // Run Music On Hold
                        Moh app = new Moh(s_config, m_fses, parameters);
                        app.run() ;
                    } else {
                        // Nothing else to run...
                        LOG.warn("Cannot determine which application to run from action="+ action);
                    }
                    m_fses.invoke(new Hangup(m_fses));
                } else {
                    LOG.info("SipXivr::run Bridging the call");
                    //setting proxy media for fax application
                    if (action.equals("faxrx")){
                        new Set(m_fses, "proxy_media","true").go();
                    }
                    Bridge app = new Bridge(s_config, m_fses);
                    app.run();
                }
            }
        } catch (DisconnectException e) {
            LOG.info("SipXivr::run Far end hungup.");
        } catch (Throwable t) {
            LOG.error("SipXivr::run", t);
        } finally {
            try {
                m_fses.close();
            } catch (IOException e) {
                // Nothing to do, no where to go home...
            }
        }

        LOG.debug("SipXivr::run Ending SipXivr thread with client " + m_clientSocket);
    }


    /**
     * Load the configuration from the sipxivr.properties file.
     * Wait for FreeSWITCH to make a TCP connection to s_eventSocketPort.
     * Spawn off a thread to handle each connection.
     *
     * @throws Throwable
     */
    static void init() throws Throwable {
        int eventSocketPort;

        // Load the configuration
        s_config = IvrConfiguration.get();

        // Configure log4j
        Properties props = new Properties();
        props.setProperty("log4j.rootLogger", "warn, file");
        props.setProperty("log4j.logger.org.sipfoundry.sipxivr", SipFoundryLayout
                .mapSipFoundry2log4j(s_config.getLogLevel()).toString());
        //props.setProperty("log4j.logger.org.mortbay", "debug");
        props.setProperty("log4j.appender.file", "org.sipfoundry.commons.log4j.SipFoundryAppender");
        props.setProperty("log4j.appender.file.File", s_config.getLogFile());
        props.setProperty("log4j.appender.file.layout", "org.sipfoundry.commons.log4j.SipFoundryLayout");
        props.setProperty("log4j.appender.file.layout.facility", "sipXivr");
        PropertyConfigurator.configure(props);

        //Start conference thread to listen to conferences
        ConfBasicThread confThread = new ConfBasicThread();
        confThread.setConfConfiguration(s_config);
        confThread.start();

        // Create Web Server
        WebServer webServer = new WebServer(s_config);
        // add MWI servlet on /mwi
        webServer.addServlet("mwistatus", "/mwi", Mwistatus.class.getName());
        webServer.addServlet("mailbox", "/mailbox/*", MailboxServlet.class.getName());
        webServer.addServlet("recording", "/recording/*", ConfRecordStatus.class.getName());
        webServer.addServlet("conference", "/conference/*", ConferenceServlet.class.getName());
        // Start it up
        webServer.start();

        ExtMailStore.Initialize();

        // Set up emailer background threads
        Emailer.init(s_config);

        eventSocketPort = s_config.getEventSocketPort();
        LOG.info("Starting SipXivr listening on port " + eventSocketPort);
        ServerSocket serverSocket = new ServerSocket(eventSocketPort);
        for (;;) {
            Socket client = serverSocket.accept();
            SipXivr ivr = new SipXivr(client);
            Thread thread = new Thread(ivr);
            thread.start();
        }
    }

    /**
     * Main entry point for sipXivr
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
