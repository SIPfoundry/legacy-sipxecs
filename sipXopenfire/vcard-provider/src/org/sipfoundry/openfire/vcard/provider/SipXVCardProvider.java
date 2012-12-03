/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.vcard.provider;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.net.ConnectException;
import java.util.Properties;

import org.apache.commons.io.IOUtils;
import org.apache.log4j.Logger;
import org.dom4j.Element;
import org.jivesoftware.openfire.provider.VCardProvider;
import org.jivesoftware.openfire.vcard.DefaultVCardProvider;
import org.jivesoftware.openfire.vcard.VCardManager;
import org.jivesoftware.util.AlreadyExistsException;
import org.jivesoftware.util.NotFoundException;
import org.jivesoftware.util.cache.Cache;
import org.jivesoftware.util.cache.CacheFactory;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.util.UnfortunateLackOfSpringSupportFactory;
import org.sipfoundry.openfire.vcard.synchserver.Util;
import org.sipfoundry.openfire.vcard.synchserver.VCardRpcServer;

/**
 * <p>
 * A vCard provider which uses sipX as user profile data source
 * </p>
 */
public class SipXVCardProvider implements VCardProvider {

    /**
     * The name of the avatar element (<tt>&lt;PHOTO&gt;</tt>) in the vCard XML.
     */
    static final String PLUGIN_CONFIG_FILENAME = "/config.properties";
    static final String MONGO_CLIENT_CONFIG = "/mongo-client.ini";
    static final String PROP_SIPX_CONF_DIR = "sipxpbx.conf.dir";
    static final String PROP_CONFIG_HOST_NAME = "CONFIG_HOSTS";
    static final String PROP_SECRET = "SHARED_SECRET";
    static final String DEFAULT_DOMAIN_NAME = "localhost";
    static final String DEFAULT_SECRET = "unknown";
    static final String MODIFY_METHOD = "PUT";
    static final String QUERY_METHOD = "GET";
    static final String PA_USER = "mybuddy";
    static final String AVATAR_ELEMENT = "PHOTO";
    static final int MAX_ATTEMPTS = 12; // Try 12 times at most when connects to sipXconfig
    static final int ATTEMPT_INTERVAL = 5000; // 5 seconds
    static long ID_index = 0;
    private static Logger logger = Logger.getLogger(SipXVCardProvider.class);

    private final Cache<String, Element> vcardCache;
    private final DefaultVCardProvider defaultProvider;

    public SipXVCardProvider() {
        super();

        defaultProvider = new DefaultVCardProvider();

        try {
            UnfortunateLackOfSpringSupportFactory.initialize();
            if (new File("/tmp/sipx.properties").exists()) {
                System.getProperties()
                        .load(new FileInputStream(new File("/tmp/sipx.properties")));
            }

        } catch (Exception e) {
            logger.error(e);
        }

        String cacheName = "SipXVCardCache";
        vcardCache = CacheFactory.createCache(cacheName);

        logger.info(this.getClass().getName() + " starting XML RPC server ...");
        try {
            VCardRpcServer vcardRpcServer = new VCardRpcServer(ContactInfoHandlerImp.class);
            vcardRpcServer.start();
            logger.info(this.getClass().getName() + " initialized");
        } catch (Exception ex) {
            logger.error(ex);
        }

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

    @Override
    synchronized public void deleteVCard(String username) {
        vcardCache.remove(username);
        defaultProvider.deleteVCard(username);

        // Refill the cache
        Element vCard = cacheVCard(username);
        try {
            Util.updateAvatar(username, vCard);
        } catch (Exception e) {
           logger.error("Cannot send update Avatar notification", e);
        }
    }

    @Override
    public Element createVCard(String username, Element element) {
        Element vcard = null;
        try {
            defaultProvider.deleteVCard(username);
            vcard = defaultProvider.createVCard(username, element);
        } catch (AlreadyExistsException e) {
            e.printStackTrace();
            logger.error("AlreadyExistsException even afer delete is called!");
            if (username.compareToIgnoreCase(PA_USER) == 0) {
                return defaultProvider.loadVCard(username);
            }

        }

        if (username.compareToIgnoreCase(PA_USER) == 0) {
            return vcard;
        }

        return updateVCard(username, element);
    }

    synchronized Element cacheVCard(String username) {
        Element vCardElement = null;
        User user = UnfortunateLackOfSpringSupportFactory.getValidUsers().getUserByJid(username);
        if (user != null) {
            Element avatarFromDB = getAvatarCopy(defaultProvider.loadVCard(username));
            vCardElement = RestInterface.buildVCardFromXMLContactInfo(user, avatarFromDB);
            if (vCardElement != null) {
                vcardCache.remove(username);
                vcardCache.put(username, vCardElement);
            } else {
                logger.error("In cacheVCard buildVCardFromXMLContactInfo failed! ");
            }
        } else {
            logger.error("In cacheVCard Failed to find peer SIP user account for XMPP user " + username);
        }

        return vCardElement;
    }

    /**
     * Loads the vCard using the SipX vCard Provider first On failure, attempt to load it from
     * database.
     *
     */
    @Override
    public Element loadVCard(String username) {
        synchronized (username.intern()) {
            if (username.compareToIgnoreCase(PA_USER) == 0) {
				return defaultProvider.loadVCard(username);
			}

            return getVCard(username);
        }
    }

    /**
     * Updates the vCard both in SipX and in the database.
     */
    @Override
    public Element updateVCard(String username, Element vCardElement) {
        if (username.compareToIgnoreCase(PA_USER) == 0) {
            try {
                return defaultProvider.updateVCard(username, vCardElement);
            } catch (NotFoundException e) {
                e.printStackTrace();
                logger.error("update " + PA_USER + "'s vcard failed!");
                return null;
            }
        }

        try {
            defaultProvider.updateVCard(username, vCardElement);
        } catch (NotFoundException e) {
            try {
                defaultProvider.createVCard(username, vCardElement);
            } catch (AlreadyExistsException e1) {
                logger.error("Failed to create vcard due to existing vcard found");
                e1.printStackTrace();
            }
            e.printStackTrace();
        }

        try {
            String sipUserName = getAORFromJABBERID(username);
            if (sipUserName != null) {

                int attempts = 0;
                boolean tryAgain;
                do {
                    tryAgain = false;
                    try {
                        RestInterface.sendRequest(MODIFY_METHOD, vCardElement);
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

                if (attempts >= MAX_ATTEMPTS) {
					logger.error("Failed to update contact info for user " + username + ", sipXconfig might be down");
				}

                Element vcardAfterUpdate = cacheVCard(username);

                //If client doesn't set local avatar, use the avatar from sipx/gravatar.
                if (getAvatar(vCardElement) == null) {
                    VCardManager.getInstance().reset();
                    Util.updateAvatar(username, vcardAfterUpdate);
                }

                return vcardAfterUpdate;

			}
			logger.error("Failed to find a valid SIP account for user "	+ username);

			return vCardElement;
		}

        catch (Exception ex) {
            logger.error("updateVCard failed! " + ex.getMessage());
            return vCardElement;
        }
    }

    /**
     * Returns <tt>false</tt> to allow users to save vCards, even if only the avatar will be
     * saved.
     *
     * @return <tt>false</tt>
     */
    @Override
    public boolean isReadOnly() {
        return false;
    }

    @SuppressWarnings("resource")
	protected Properties loadProperties(String path_under_conf_dir) {

        Properties result = null;

        InputStream in = this.getClass().getResourceAsStream(PLUGIN_CONFIG_FILENAME);
        Properties properties = new Properties();

        try {
            properties.load(in);
        } catch (IOException ex) {
            logger.error(ex);
        } finally {
        	IOUtils.closeQuietly(in);
        }

        String file_path = properties.getProperty(PROP_SIPX_CONF_DIR) + path_under_conf_dir;
        logger.info("Domain config file path is  " + file_path);

        InputStream fis = null;
        try {
            fis = new FileInputStream(file_path);
            result = new Properties();
            result.load(fis);

        } catch (Exception e) {
            logger.error("Failed to read '" + file_path + "':");
            System.err.println("Failed to read '" + file_path + "':");
            e.printStackTrace(System.err);
        } finally {
        	IOUtils.closeQuietly(fis);
        }

        return result;
    }

    @SuppressWarnings("resource")
	public String getConfDir() {
        InputStream in = this.getClass().getResourceAsStream("/config.properties");
        Properties properties = new Properties();

        try {
            properties.load(in);
        } catch (IOException ex) {
            logger.error(ex);
        } finally {
        	IOUtils.closeQuietly(in);
        }

        return properties.getProperty("sipxpbx.conf.dir");
    }

    public static String getAORFromJABBERID(String jabberid) {
        try {
            User user = UnfortunateLackOfSpringSupportFactory.getValidUsers().getUserByJid(jabberid);
            if (user != null) {
                return user.getUserName();
            }

            return null;
        } catch (Exception ex) {
            logger.error("getAORFROMJABBERID exception " + ex.getMessage());
            return null;
        }
    }

    @SuppressWarnings("resource")
	public static String readXML(File file) {
    	String contents = null;
    	
        StringBuilder builder = new StringBuilder();
        BufferedReader reader = null;

        try {
            reader = new BufferedReader(new FileReader(file));

            String line;
            while ((line = reader.readLine()) != null) {
                builder.append(line);
            }

            contents = builder.toString().replaceAll("xmlns=", "dummy="); // dom4j parser doesn't like
            // xmlns in the root element
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
        	IOUtils.closeQuietly(reader);
        }
        
        return contents;
    }

    /**
     * Loads the vCard using the LDAP vCard Provider and re-adds the avatar from the database.
     *
     * @param username
     * @return LDAP vCard re-added avatar element
     */
    synchronized static Element mergeAvatar(String username, Element vcardFromSipX, Element vcardFromDB) {

        logger.info("merge avatar for user '" + username + "' ...");

        // get the vcard from ldap
        // Element vCardElement = super.loadVCard(username);

        // only add avatar if it doesn't exist already
        if (vcardFromDB != null && vcardFromDB.element(AVATAR_ELEMENT) != null) {
            Element avatarElement = getAvatarCopy(vcardFromDB);

            if (avatarElement != null) {
                if (getAvatar(vcardFromSipX) != null) {
                    vcardFromSipX.remove(getAvatar(vcardFromSipX));
                }
                vcardFromSipX.add(avatarElement);
                logger.info("Avatar merged from DB into sipX vCard");
            } else {
                logger.info("No vCard found in database");
            }
        }

        return vcardFromSipX;
    }

    protected static Element getAvatarCopy(Element vcard) {
        Element avatarElement = null;
        if (vcard != null) {
            Element photoElement = vcard.element(AVATAR_ELEMENT);
            if (photoElement != null) {
				avatarElement = photoElement.createCopy();
			}
        }

        return avatarElement;
    }

    protected static Element getAvatar(Element vcard) {
        Element avatarElement = null;
        if (vcard != null) {
            return vcard.element(AVATAR_ELEMENT);
        }
        return avatarElement;
    }



}
