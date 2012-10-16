/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.vcard.synchserver;

import java.io.IOException;

import org.apache.log4j.Logger;
import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.server.PropertyHandlerMapping;
import org.apache.xmlrpc.server.XmlRpcServer;
import org.apache.xmlrpc.server.XmlRpcServerConfigImpl;
import org.apache.xmlrpc.webserver.WebServer;
import org.sipfoundry.openfire.vcard.synchserver.ContactInfoHandler;
import org.xml.sax.SAXException;

public class VCardRpcServer {
    Class<? extends ContactInfoHandler> m_contactInfoHandler;
    final static String NAME_RPC_HANDLER = "ContactInfoHandler";
    private static Logger logger = Logger.getLogger(VCardRpcServer.class);

    public VCardRpcServer(Class<? extends ContactInfoHandler> contactInfoHandler) {
        m_contactInfoHandler = contactInfoHandler;
    }

    public void start() {
        try {
            WebServer webServer = new WebServer(getRpcPort());
            XmlRpcServer xmlRpcServer = webServer.getXmlRpcServer();
            PropertyHandlerMapping phm = new PropertyHandlerMapping();
            phm.setVoidMethodEnabled(true);

            phm.addHandler(NAME_RPC_HANDLER, m_contactInfoHandler);
            xmlRpcServer.setHandlerMapping(phm);

            XmlRpcServerConfigImpl serverConfig = (XmlRpcServerConfigImpl) xmlRpcServer.getConfig();
            serverConfig.setEnabledForExtensions(true);
            webServer.start();
        } catch (XmlRpcException e) {
            logger.info("In VcardRpcServer, XmlRpcException " + e.getMessage());
        } catch (IOException e) {
            logger.info("In VcardRpcServer, IOException " + e.getMessage());
        } catch (Exception e) {
            logger.info("In VcardRpcServer, Exception " + e.getMessage());
        }

    }

    private int getRpcPort() throws SAXException, IOException {
        String configurationFile = System.getProperty("conf.dir", "/etc/sipxpbx") + "/sipxopenfire.xml";
        VcardConfigurationParser parser = new VcardConfigurationParser();
        VcardConfig vcardConfig  = parser.parse( "file://" + configurationFile );
        return vcardConfig.getOpenfireXmlRpcVcardPort();
    }

}
