/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.vcard.provider;

import java.io.IOException;

import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.server.PropertyHandlerMapping;
import org.apache.xmlrpc.server.XmlRpcServer;
import org.apache.xmlrpc.server.XmlRpcServerConfigImpl;
import org.apache.xmlrpc.webserver.WebServer;
import org.jivesoftware.util.Log;

public class VCardRpcServer {
    int m_serverPort;
    final static String NAME_RPC_HANDLER = "ContactInfoHandler";

    public VCardRpcServer(int serverPort) {
        m_serverPort = serverPort;
    }

    public void start() {
        try {
            WebServer webServer = new WebServer(m_serverPort);
            XmlRpcServer xmlRpcServer = webServer.getXmlRpcServer();
            PropertyHandlerMapping phm = new PropertyHandlerMapping();
            phm.setVoidMethodEnabled(true);

            phm.addHandler(NAME_RPC_HANDLER, ContactInfoHandlerImp.class);
            xmlRpcServer.setHandlerMapping(phm);

            XmlRpcServerConfigImpl serverConfig = (XmlRpcServerConfigImpl) xmlRpcServer.getConfig();
            serverConfig.setEnabledForExtensions(true);
            webServer.start();
        } catch (XmlRpcException e) {
            Log.info("In VcardRpcServer, XmlRpcException " + e.getMessage());
        } catch (IOException e) {
            Log.info("In VcardRpcServer, IOException " + e.getMessage());
        } catch (Exception e) {
            Log.info("In VcardRpcServer, Exception " + e.getMessage());
        }

    }

}
