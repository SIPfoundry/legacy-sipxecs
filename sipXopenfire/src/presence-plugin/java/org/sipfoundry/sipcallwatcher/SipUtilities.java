/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */

package org.sipfoundry.sipcallwatcher;

import java.io.InputStream;
import java.util.Properties;

import javax.sip.header.ServerHeader;
import javax.sip.header.UserAgentHeader;

import org.apache.log4j.Logger;

@SuppressWarnings("unchecked")
class SipUtilities 
{
    private static Logger logger = Logger.getLogger(SipUtilities.class);
    static UserAgentHeader userAgent;
    static ServerHeader serverHeader;
    private static final String CONFIG_PROPERTIES = "config.properties";
    
    
    /**
     * Create the UA header.
     * 
     * @return
     */
    static UserAgentHeader createUserAgentHeader() {
        if (userAgent != null) {
            return userAgent;
        } else {
            try {
                InputStream cfg = SipUtilities.class.getClassLoader().getResourceAsStream(
                        CONFIG_PROPERTIES);
                if (cfg != null) {
                    Properties configProperties = new Properties();

                    configProperties.load(cfg);
                    userAgent = (UserAgentHeader) CallWatcher.getSipStackBean().getHeaderFactory().createHeader(
                            UserAgentHeader.NAME, String.format(
                                    "sipXecs/%s sipXecs/sipxcallwatcher (Linux)", configProperties
                                            .get("version")));
                    cfg.close();
                    return userAgent;
                } else {
                    userAgent = (UserAgentHeader) CallWatcher.getSipStackBean().getHeaderFactory().createHeader(
                            UserAgentHeader.NAME, String.format(
                                    "sipXecs/%s sipXecs/sipxcallwatcher (Linux)", "1.0"));
                    logger.warn("Creating Default User Agent Header");
                    return userAgent;

                }
            } catch (Exception e) {
                throw new CallWatcherException("unexpected exception creating UserAgent header",
                        e);

            }
        }
    }

    /**
     * Possibly create and return the default Server header.
     * 
     * @return
     */
    static ServerHeader createServerHeader() {
        if (serverHeader != null) {
            return serverHeader;
        } else {
            try {
                InputStream cfg = SipUtilities.class.getClassLoader().getResourceAsStream(
                        CONFIG_PROPERTIES);
                if (cfg != null) {
                    Properties configProperties = new Properties();
                    configProperties.load(cfg);
                    serverHeader = (ServerHeader) CallWatcher.getSipStackBean().getHeaderFactory().createHeader(
                            ServerHeader.NAME, String.format(
                                    "sipXecs/%s sipXecs/sipxbridge (Linux)", configProperties
                                            .get("version")));
                    cfg.close();
                    return serverHeader;
                } else {
                    serverHeader = (ServerHeader) CallWatcher.getSipStackBean().getHeaderFactory().createHeader(
                            ServerHeader.NAME, String.format(
                                    "sipXecs/%s sipXecs/sipxbridge (Linux)", "xxxx.yyyy"));
                    logger.warn("Creating Default Server Header");
                    return serverHeader;
                }
            } catch (Exception e) {
                throw new CallWatcherException("unexpected exception creating UserAgent header",
                        e);

            }
        }
    }
}
