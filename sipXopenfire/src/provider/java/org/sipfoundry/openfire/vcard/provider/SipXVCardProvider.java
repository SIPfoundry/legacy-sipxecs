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

import java.io.File;
import java.io.IOException;
import java.io.StringReader;
import java.io.InputStream;
import java.util.Properties;
import java.util.List;
import java.util.Iterator;
import java.lang.reflect.Method;
import java.net.URL;
import java.security.MessageDigest;
import java.net.URLClassLoader;

import org.dom4j.Element;
import org.dom4j.Document;
import org.dom4j.Node;
import org.dom4j.DocumentHelper;
import org.dom4j.io.SAXReader;
import org.dom4j.Namespace;
import org.dom4j.dom.DOMElement;
import org.jivesoftware.util.AlreadyExistsException;
import org.jivesoftware.util.Log;
import org.xmpp.packet.IQ;
import org.xmpp.packet.Message;
import org.jivesoftware.util.NotFoundException;
import org.jivesoftware.openfire.XMPPServer;
import org.jivesoftware.openfire.XMPPServerInfo;
import org.jivesoftware.openfire.IQRouter;
import org.jivesoftware.openfire.pubsub.PubSubModule;
import org.jivesoftware.openfire.SessionManager;
import org.jivesoftware.openfire.user.UserManager;
import org.jivesoftware.openfire.vcard.VCardProvider;
import org.jivesoftware.openfire.vcard.DefaultVCardProvider;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.DataInputStream;
import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.FileNotFoundException;

/**
 * <p>
 * A vCard provider which uses sipX as user profile data source
 * </p>
 */
public class SipXVCardProvider implements VCardProvider {

    /**
     * The name of the avatar element (<tt>&lt;PHOTO&gt;</tt>) in the vCard XML.
     */
    private static final String DOMAIN_CONFIG_FILENAME = "/domain-config";
    private static final String PLUGIN_CONFIG_FILENAME = "/config.properties";
    private static final String PROP_SIPX_CONF_DIR = "sipxpbx.conf.dir";
    private static final String PROP_DOMAIN_NAME = "SIP_DOMAIN_NAME";
    private static final String PROP_SECRET = "SHARED_SECRET";
    private static final String DEFAULT_DOMAIN_NAME = "localhost";
    private static final String DEFAULT_SECRET = "unknown";
    private static final String MODIFY_METHOD = "PUT";
    private static final String QUERY_METHOD = "GET";
    private String m_SipDomainName;
    private String m_SharedSecret;

    /**
     * The default vCard provider is used to handle the vCard in the database.
     */
    private DefaultVCardProvider defaultProvider = null;

    /**
     * Initializes the SipX vCard provider and then creates an instance of the default vCard
     * provider to use for database interaction.
     */
    public SipXVCardProvider() {
        super();

        defaultProvider = new DefaultVCardProvider();

        Properties domain_config = loadProperties(DOMAIN_CONFIG_FILENAME);
        if (null != domain_config) {

            m_SipDomainName = domain_config.getProperty(PROP_DOMAIN_NAME, DEFAULT_DOMAIN_NAME);
            m_SharedSecret = domain_config.getProperty(PROP_SECRET, DEFAULT_SECRET);
        }

        Log.info("Domain name is " + m_SipDomainName + " ShardSecret is " + m_SharedSecret);

        initTLS();

        Log.info(this.getClass().getName() + " initialized");

    }

    /**
     * SipXconfig Does NOT support "delete user profile". So only the big cache (database) record
     * is removed.
     */

    public void deleteVCard(String username) {
        defaultProvider.deleteVCard(username);
    }

    public Element createVCard(String username, Element element) {
        try {
            String sipUserName = getAORFromJABBERID(username);
            if (sipUserName != null) {
                String resp = RestInterface.sendRequest(MODIFY_METHOD, m_SipDomainName,
                        sipUserName, m_SharedSecret, element);
                Log.debug("response from createVCard is " + resp);
            } else {
                Log.debug("Cannot find sip user account for user " + username);
            }

            return defaultProvider.createVCard(username, element);
        } catch (AlreadyExistsException e) {
            Log.info("vCard already exists for user '" + username + "'");
            return element;
        }
    }

    /**
     * Loads the vCard using the SipX vCard Provider first On failure, attempt to load it from
     * database.
     *
     */
    public Element loadVCard(String username) {
        Element vCardElement = null;
        try {
            String sipUserName = getAORFromJABBERID(username);
            if (sipUserName != null) {
                String resp = RestInterface.sendRequest(QUERY_METHOD, m_SipDomainName,
                        sipUserName, m_SharedSecret, null);
                if (resp != null) {
                    vCardElement = RestInterface.buildVCardFromXMLContactInfo(resp);
                    if (vCardElement != null) {
                        defaultProvider.deleteVCard(username);
                        defaultProvider.createVCard(username, vCardElement);
                        // IQAvatar(username, vCardElement.element("PHOTO").element("BINVAL"));
                        return vCardElement;
                    }
                }

                Log.info("VCard for user " + username + " loaded from sipX failed!");
            } else {
                Log.info("Failed to find SIP user account for user " + username);
            }

            return defaultProvider.loadVCard(username);

        } catch (AlreadyExistsException ex) {
            return vCardElement;
        }

        catch (Exception e) {
            return defaultProvider.loadVCard(username);
        }

    }

    /**
     * Updates the vCard both in SipX and in the database.
     */
    public Element updateVCard(String username, Element vCardElement) throws NotFoundException {

        try {
            String sipUserName = getAORFromJABBERID(username);
            if (sipUserName != null) {
                RestInterface.sendRequest(MODIFY_METHOD, m_SipDomainName, sipUserName,
                        m_SharedSecret, vCardElement);

                return loadVCard(username);

            } else {
                Log.info("Failed to find SIP account for user " + username);
                return defaultProvider.updateVCard(username, vCardElement);
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
                // SAXReader reader = new SAXReader();
                // Document doc = reader.read(accountFile);
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

    public static void IQAvatar(String username, Element binValElement) {

        if (binValElement == null)
            return;

        try {

            String avatarStr = binValElement.getText();

            XMPPServer server = XMPPServer.getInstance();
            SessionManager smgr = server.getSessionManager();
            UserManager umgr = server.getUserManager();
            PubSubModule psm = server.getPubSubModule();
            XMPPServerInfo info = server.getServerInfo();
            String aor = username + "@" + info.getXMPPDomain();
            String itemId = getItemId(avatarStr.getBytes());

            Log.info("the itemid after sha1 is " + itemId);

            StringBuilder xbuilder = new StringBuilder("<iq type='set' from='TBD' id='TBD'>");
            xbuilder.append("<pubsub xmlns='http://jabber.org/protocol/pubsub'>");
            xbuilder.append("<publish node='urn:xmpp:avatar:data'>");
            xbuilder.append("<item id='");
            xbuilder.append(itemId);
            xbuilder.append("'>");
            xbuilder.append("<data xmlns='urn:xmpp:avatar:data'>");
            xbuilder.append(avatarStr);
            xbuilder.append("</data>");
            xbuilder.append("</item>");
            xbuilder.append("</publish>");
            xbuilder.append("</pubsub>");
            xbuilder.append("</iq>");

            String xmlstr1 = xbuilder.toString();
            SAXReader sreader = new SAXReader();

            Document avatarDoc = sreader.read(new StringReader(xmlstr1));
            Element rootElement = avatarDoc.getRootElement();

            IQ avatar = new IQ(rootElement);
            avatar.setFrom(aor);
            avatar.setID(aor + "_publish1");

            psm.process(avatar);

            StringBuilder builder2 = new StringBuilder("<iq type='set' from='TBD' id='TBD'>");
            builder2.append("<pubsub xmlns='http://jabber.org/protocol/pubsub'>");
            builder2.append("<publish node='urn:xmpp:avatar:metadata'>");
            builder2.append("<item id='");
            builder2.append(itemId + "_" + aor);
            builder2.append("'>");
            builder2.append("<metadata xmlns='urn:xmpp:avatar:metadata'>");
            builder2.append("<info ");
            builder2.append("id='");
            builder2.append(itemId);
            builder2.append("'");
            builder2.append("type='image/png'");
            builder2.append("</metadata>");
            builder2.append("</item>");
            builder2.append("</publish>");
            builder2.append("</pubsub>");
            builder2.append("<iq>");

            String xmlstr2 = builder2.toString();

            avatarDoc = sreader.read(new StringReader(xmlstr2));
            rootElement = avatarDoc.getRootElement();

            avatar = new IQ(rootElement);
            avatar.setFrom(aor);
            avatar.setID(aor + "_publish2");
            psm.process(avatar);

            Log.info("psm process message done!");
        } catch (Exception ex) {
            Log.error(ex.getMessage());
        }
    }

    public static String getItemId(byte[] avatarBytes) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-1");
            md.update(avatarBytes);
            // md.update( int ) processes only the low order 8-bits. It actually expects an
            // unsigned byte.
            byte[] digest = md.digest();

            return byteArrayToString(digest);
        }
        /*
         * catch(NoSuchAlgorithmException ex) { Log.error("no such algorithm of SHA " +
         * ex.getMessage()); return ""; }
         */
        catch (Exception ex) {
            Log.error(ex.getMessage());
            return "thisisafakeitemid";
        }
    }

    public static String byteArrayToString(byte[] bytes) {
        StringBuilder sb = new StringBuilder(bytes.length * 2);
        for (int i = 0; i < bytes.length; i++) {
            sb.append(String.format("%02x", bytes[i]));
        }
        return sb.toString();
    }

}
