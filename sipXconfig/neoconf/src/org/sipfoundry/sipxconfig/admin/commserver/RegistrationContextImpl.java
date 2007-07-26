/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.text.MessageFormat;
import java.util.Collections;
import java.util.List;

import org.apache.commons.digester.Digester;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.ImdbXmlHelper;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.RegistrationItem;
import org.xml.sax.SAXException;

public class RegistrationContextImpl implements RegistrationContext {
    public static final Log LOG = LogFactory.getLog(RegistrationContextImpl.class);

    private static final String REGISTRATION_URL = "https://{0}:{1}/sipdb/registration.xml";

    private String m_server;

    private String m_port;

    /**
     * @see org.sipfoundry.sipxconfig.admin.commserver.RegistrationContext#getRegistrations()
     */
    public List<RegistrationItem> getRegistrations() {
        try {
            URL url = new URL(getUrl());
            InputStream is = url.openStream();
            List<RegistrationItem> registrations = getRegistrations(is);
            return registrations;
        } catch (FileNotFoundException e) {
            // we are handling this separately - server returns FileNotFound even if everything is
            // OK but we have no registrations present
            LOG.info("No registrations found on the server" + e.getMessage());
            return Collections.<RegistrationItem>emptyList();
        } catch (IOException e) {
            LOG.error("Retrieving active registrations failed", e);
            return Collections.<RegistrationItem>emptyList();
        } catch (SAXException e) {
            throw new RuntimeException(e);
        }
    }
        
    List<RegistrationItem> getRegistrations(InputStream is) throws IOException, SAXException {
        Digester digester = ImdbXmlHelper.configureDigester(RegistrationItem.class);
        return (List<RegistrationItem>) digester.parse(is);
    }

    String getUrl() {
        Object[] params = {
            m_server, m_port
        };
        return MessageFormat.format(REGISTRATION_URL, params);
    }

    public void setPort(String port) {
        m_port = port;
    }

    public void setServer(String server) {
        m_server = server;
    }
}
