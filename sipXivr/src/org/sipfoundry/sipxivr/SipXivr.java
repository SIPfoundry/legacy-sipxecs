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
import java.net.Socket;
import java.util.Hashtable;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.Answer;
import org.sipfoundry.commons.freeswitch.DisconnectException;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocketInterface;
import org.sipfoundry.commons.freeswitch.Hangup;
import org.sipfoundry.commons.freeswitch.Set;
import org.sipfoundry.sipxivr.eslrequest.EslRequestScopeRunnable;
import org.sipfoundry.sipxivr.eslrequest.EslRequestApp;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;

public abstract class SipXivr extends EslRequestScopeRunnable implements ApplicationContextAware {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private Socket m_clientSocket;
    private ApplicationContext m_context;

    protected abstract FreeSwitchEventSocketInterface getFsEventSocket();

    protected abstract EslRequestApp getBridgeApp();

    /**
     * Determine what to do based on the SIP request.
     * 
     * Run the autoattendant if action=autoattendant Otherwise, hang up.
     */
    @Override
    public void runEslRequest() {
        LOG.debug("SipXivr::run Starting SipXivr thread with client " + m_clientSocket);

        FreeSwitchEventSocketInterface fses = getFsEventSocket();
        try {
            if (fses.connect(m_clientSocket, null)) {
                LOG.info(String.format("SipXivr::run Accepting call-id %s from %s to %s",
                        fses.getVariable("variable_sip_call_id"), fses.getVariable("variable_sip_from_uri"),
                        fses.getVariable("variable_sip_req_uri")));

                Hashtable<String, String> parameters = extractCallParameters(fses);
                String action = parameters.get("action");
                String uuid = parameters.get("uuid");
                if (uuid != null) {

                    fses.invoke(new Answer(fses));

                    // identify and run proper app to handle action
                    if (action != null) {
                        EslRequestApp sipXivrApp = m_context.getBean(action, EslRequestApp.class);
                        sipXivrApp.run(parameters);
                    } else {
                        LOG.warn("Cannot determine which application to run as the action parameter is missing.");
                    }

                    fses.invoke(new Hangup(fses));
                } else {
                    LOG.info("SipXivr::run Bridging the call");
                    // setting proxy media for fax application
                    if (action.equals("faxrx")) {
                        new Set(fses, "proxy_media", "true").go();
                    }
                    getBridgeApp().run(parameters);
                }
            }
        } catch (DisconnectException e) {
            LOG.info("SipXivr::run Far end hungup.");
        } catch (Throwable t) {
            LOG.error("SipXivr::run", t);
        } finally {
            try {
                fses.close();
            } catch (IOException e) {
                // Nothing to do, no where to go home...
            }
        }

        LOG.debug("SipXivr::run Ending SipXivr thread with client " + m_clientSocket);
    }

    private Hashtable<String, String> extractCallParameters(FreeSwitchEventSocketInterface fses) {
        String sipReqParams = fses.getVariable("variable_sip_req_params");
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
        return parameters;
    }

    public void setClient(Socket client) {
        m_clientSocket = client;
    }

    @Override
    public void setApplicationContext(ApplicationContext context) {
        m_context = context;
    }
}
