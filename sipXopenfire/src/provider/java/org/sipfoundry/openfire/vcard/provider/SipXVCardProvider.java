/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.openfire.vcard.provider;

import java.io.BufferedInputStream;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.net.ConnectException;
import java.util.Iterator;
import java.util.Properties;

import org.dom4j.Document;
import org.dom4j.DocumentHelper;
import org.dom4j.Element;
import org.dom4j.Node;
import org.jivesoftware.openfire.vcard.VCardProvider;
import org.jivesoftware.openfire.vcard.DefaultVCardProvider;
import org.jivesoftware.util.AlreadyExistsException;
import org.jivesoftware.util.Log;
import org.jivesoftware.util.NotFoundException;
import org.jivesoftware.util.cache.Cache;
import org.jivesoftware.util.cache.CacheFactory;
import org.sipfoundry.openfire.vcard.provider.RestInterface;

/**
 * <p>
 * A vCard provider which uses sipX as user profile data source
 * </p>
 */
public class SipXVCardProvider implements VCardProvider {

    /**
     * The name of the avatar element (<tt>&lt;PHOTO&gt;</tt>) in the vCard XML.
     */
    static final String DOMAIN_CONFIG_FILENAME = "/domain-config";
    static final String PLUGIN_CONFIG_FILENAME = "/config.properties";
    static final String PROP_SIPX_CONF_DIR = "sipxpbx.conf.dir";
    static final String PROP_CONFIG_HOST_NAME = "CONFIG_HOSTS";
    static final String PROP_SECRET = "SHARED_SECRET";
    static final String DEFAULT_DOMAIN_NAME = "localhost";
    static final String DEFAULT_SECRET = "unknown";
    static final String MODIFY_METHOD = "PUT";
    static final String QUERY_METHOD = "GET";
    static final String PA_USER = "MyAssistant";
    static final int    MAX_ATTEMPTS = 12; //Try 12 times at most when connects to sipXconfig
    static final int    ATTEMPT_INTERVAL = 5000; // 5 seconds
    private String m_ConfigHostName;
    private String m_SharedSecret;
    static long ID_index = 0;

    private Cache<String, Element> vcardCache;
    private DefaultVCardProvider defaultProvider;

    public SipXVCardProvider() {
        super();

        defaultProvider = new DefaultVCardProvider();

        Properties domain_config = loadProperties(DOMAIN_CONFIG_FILENAME);
        if (null != domain_config) {

            m_ConfigHostName = domain_config.getProperty(PROP_CONFIG_HOST_NAME, DEFAULT_DOMAIN_NAME).split(" ")[0];
            m_SharedSecret = domain_config.getProperty(PROP_SECRET, DEFAULT_SECRET);
        }

        Log.info("CONFIG_HOSTS is " + m_ConfigHostName);

        initTLS();

        String cacheName = "SipXVCardCashe";
        vcardCache = CacheFactory.createCache(cacheName);

        Log.info(this.getClass().getName() + " initialized");

    }

    synchronized public Element getVCard(String username) {
        Element vcard = vcardCache.get(username);
        if (vcard == null) {
            return cacheVCard(username);
        }
        return vcard;
    }

    /**
     * SipXconfig Does NOT support "delete user profile". So only the big cache (database) record
     * is removed.
     */

    public void deleteVCard(String username) {
        if(username.compareToIgnoreCase(PA_USER) == 0)
            deleteVCard(username);
    }

    public Element createVCard(String username, Element element) {

        if (username.compareToIgnoreCase(PA_USER) == 0) {
            try {
                return defaultProvider.createVCard(username, element);
            } catch (AlreadyExistsException e) {
                e.printStackTrace();
                return null;
            }
        }

        return updateVCard(username, element);
    }

    synchronized Element cacheVCard(String username) {
        Element vCardElement = null;
        String sipUserName = getAORFromJABBERID(username);
        if (sipUserName != null) {
            String resp = null;

            int attempts = 0;
            boolean tryAgain;
            do {
                tryAgain = false;
                try {
                    resp = RestInterface.sendRequest(QUERY_METHOD, m_ConfigHostName, sipUserName,
                            m_SharedSecret, null);
                } catch (ConnectException e) {
                    try {
                        Thread.sleep(ATTEMPT_INTERVAL);
                    } catch (InterruptedException e1) {
                        e1.printStackTrace();
                    }
                    attempts++;
                    tryAgain = true;
                }
            } while (resp == null && attempts < MAX_ATTEMPTS && tryAgain);

            if (resp != null) {
                vCardElement = RestInterface.buildVCardFromXMLContactInfo(sipUserName, resp);
                if (vCardElement != null) {
                    vcardCache.remove(username);
                    vcardCache.put(username, vCardElement);
                } else {
                    Log.error("In cacheVCard buildVCardFromXMLContactInfo failed! ");
                }
            } else {
                Log.error("In cacheVCard, No valid response from sipXconfig, " + username
                        + "'s vcard info not loaded!");
            }
        } else {
            Log.error("In cacheVCard Failed to find peer SIP user account for XMPP user "
                    + username);
        }

        return vCardElement;
    }

    /**
     * Loads the vCard using the SipX vCard Provider first On failure, attempt to load it from
     * database.
     *
     */
    public Element loadVCard(String username) {
        if (username.compareToIgnoreCase(PA_USER) == 0)
            return defaultProvider.loadVCard(username);

        return getVCard(username);
    }

    /**
     * Updates the vCard both in SipX and in the database.
     *////
    public Element updateVCard(String username, Element vCardElement) {
        if (username.compareToIgnoreCase(PA_USER) == 0) {
            try {
                return defaultProvider.updateVCard(username, vCardElement);
            } catch (NotFoundException e) {
                e.printStackTrace();
                return null;
            }
        }

        try {
            String sipUserName = getAORFromJABBERID(username);
            if (sipUserName != null) {

                int attempts = 0;
                boolean tryAgain;
                do {
                    tryAgain = false;
                    try {
                        RestInterface.sendRequest(MODIFY_METHOD, m_ConfigHostName, sipUserName, m_SharedSecret,
                                vCardElement);
                    } catch (ConnectException e) {
                        try {
                            Thread.sleep(ATTEMPT_INTERVAL);
                        } catch (Exception e1) {
                            e1.printStackTrace();
                        }
                        attempts++;
                        tryAgain = true;
                    }
                } while (attempts < MAX_ATTEMPTS && tryAgain);

                if (attempts >= MAX_ATTEMPTS)
                    Log.error("Failed to update contact info for user " + username + ", sipXconfig might be down" );

                return cacheVCard(username);

            } else {
                Log.error("Failed to find a valid SIP account for user " + username);
                return vCardElement;
            }
        }

        catch (Exception ex) {
            Log.error("updateVCard failed! " + ex.getMessage());
            return vCardElement;
        }

    }

    /**
     * Returns <tt>false</tt> to allow users to save vCards, even if only the avatar will be
     * saved.
     *
     * @return <tt>false</tt>
     */
    public boolean isReadOnly() {
        return false;
    }

    protected Properties loadProperties(String path_under_conf_dir) {

        Properties result = null;

        InputStream in = this.getClass().getResourceAsStream(PLUGIN_CONFIG_FILENAME);
        Properties properties = new Properties();

        try {
            properties.load(in);
        } catch (IOException ex) {
            Log.error(ex);
        }

        String file_path = properties.getProperty(PROP_SIPX_CONF_DIR) + path_under_conf_dir;
        Log.info("Domain config file path is  " + file_path);
        try {
            FileInputStream fis = new FileInputStream(file_path);
            result = new Properties();
            result.load(fis);

        } catch (Exception e) {
            Log.error("Failed to read '" + file_path + "':");
            System.err.println("Failed to read '" + file_path + "':");
            e.printStackTrace(System.err);
        }

        return result;
    }

    public String getConfDir() {

        Properties result = null;

        InputStream in = this.getClass().getResourceAsStream("/config.properties");
        Properties properties = new Properties();

        try {
            properties.load(in);
        } catch (IOException ex) {
            Log.error(ex);
        }

        return properties.getProperty("sipxpbx.conf.dir");
    }

    public void initTLS() {
        // Setup SSL properties so we can talk to HTTPS servers
        String path = getConfDir();
        String keyStore = path + "/ssl/ssl.keystore";
        if (System.getProperty("javax.net.ssl.keyStore") == null) {
            System.setProperty("javax.net.ssl.keyStore", keyStore);
            System.setProperty("javax.net.ssl.keyStorePassword", "changeit"); // wow
        }
        String trustStore = path + "/ssl/authorities.jks";
        if (System.getProperty("javax.net.ssl.trustStore") == null) {
            System.setProperty("javax.net.ssl.trustStore", trustStore);
            System.setProperty("javax.net.ssl.trustStoreType", "JKS");
            System.setProperty("javax.net.ssl.trustStorePassword", "changeit"); // wow
        }
    }

    public String getAORFromJABBERID(String jabberid) {
        String path = getConfDir();
        String accountFileName = path + "/xmpp-account-info.xml";

        try {
            File accountFile = new File(accountFileName);
            String xmlstr = readXML(accountFile);

            if (xmlstr != null) {
                Log.debug("after readXML " + xmlstr);
                Document doc = DocumentHelper.parseText(xmlstr);
                Element root = doc.getRootElement();
                for (Iterator i = root.elements().iterator(); i.hasNext();) {
                    Element user = (Element) i.next();
                    Node userName = user.selectSingleNode("user-name");
                    Node sipUserName = user.selectSingleNode("sip-user-name");
                    Log.debug("username = " + userName.getText());
                    Log.debug("sipUserName = " + sipUserName.getText());
                    if (userName != null && sipUserName != null) {
                        if (userName.getText().compareToIgnoreCase(jabberid) == 0) {
                            return sipUserName.getText();
                        }
                    }
                }
            }

            return null;
        } catch (Exception ex) {
            Log.error("getAORFROMJABBERID excption " + ex.getMessage());
            return null;
        }
    }

    public String readXML(File file) {

        StringBuilder builder = new StringBuilder();
        FileInputStream fis = null;
        BufferedInputStream bis = null;
        DataInputStream dis = null;

        try {
            fis = new FileInputStream(file);
            bis = new BufferedInputStream(fis);
            dis = new DataInputStream(bis);

            while (dis.available() != 0) {
                builder.append(dis.readLine());
            }

            fis.close();
            bis.close();
            dis.close();

            return builder.toString().replaceAll("xmlns=", "dummy="); // dom4j parser doesn't like
            // xmlns in the root element
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            return null;
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }
}
