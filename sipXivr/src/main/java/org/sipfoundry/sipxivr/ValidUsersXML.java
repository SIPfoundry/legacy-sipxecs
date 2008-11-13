/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxivr;

import java.io.File;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.Vector;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

/**
 * Holds the valid user data needed for the AutoAttendant, parsing from IMDB XML files
 * 
 */
public class ValidUsersXML {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private static ValidUsersXML s_current;
    private static File s_validUsersFile;
    private static long s_lastModified;

    // Mapping of letters to DTMF numbers.
    // Position of letter in letters maps to corresponding position in numbers
    private static String s_letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    private static String s_numbers = "22233344455566677778889999";

    private Vector<User> m_users;
    private HashMap<String, User> m_userNameMap;


    /**
     * Private constructor for updatable singleton
     */
    private ValidUsersXML() {
        m_userNameMap = new HashMap<String, User>();
        m_users = new Vector<User>();
    }

    /**
     * Load new ValidUsers object if the underlying properties files have changed since the last
     * time
     * 
     * @return
     */
    public static ValidUsersXML update(boolean load) {
        if (s_current == null || s_validUsersFile.lastModified() != s_lastModified) {
            s_current = new ValidUsersXML();
            if (load) {
                s_current.loadXML();
            }
        }
        return s_current;
    }

    /**
     * Load the validusers.xml file
     */
    void loadXML() {
    	LOG.info("Loading validusers.xml configuration");
        String path = System.getProperty("conf.dir");
        if (path == null) {
            LOG.fatal("Cannot get System Property conf.dir!  Check jvm argument -Dconf.dir=") ;
            System.exit(1);
        }
        
        Document validUsers = null;
        
        try {
            s_validUsersFile = new File(path + "/validusers.xml");
            s_lastModified = s_validUsersFile.lastModified();

            DocumentBuilder builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
            validUsers = builder.parse(s_validUsersFile);
            
        } catch (Throwable t) {
            LOG.fatal("Something went wrong loading the validusers.xml file.", t);
            System.exit(1);
        }
        
        walkXML(validUsers);
    }
    
    void walkXML(Document validUsers) {
        String prop = null;
        try {
            prop = "user";
            // Walk validUsers, building up m_users as we go
            NodeList users = validUsers.getElementsByTagName("user");
            for (int userNum = 0; userNum < users.getLength(); userNum++) {
                Node user = users.item(userNum);
                Node next = user.getFirstChild();
                
                User u = new User() ;
                Vector<String> aliases = new Vector<String>();
                while (next != null) {
                    if (next.getNodeType() == Node.ELEMENT_NODE) {
                        String name = next.getNodeName();
                        String text = next.getTextContent().trim();
                        if (name.contentEquals("identity")) {
                            u.setIdentity(text);
                        } else if (name.contentEquals("userName")) {
                            u.setUserName(text);
                        } else if (name.contentEquals("displayName")) {
                            u.setDisplayName(text);
                        } else if (name.contentEquals("contact")) {
                            u.setUri(text);
                        } else if (name.contentEquals("pintoken")) {
                            u.setPintoken(text);
                        } else if (name.contentEquals("inDirectory")) {
                            u.setInDirectory(Boolean.parseBoolean(text));
                        } else if (name.contentEquals("aliases")) {
                        	Node alias = next.getFirstChild() ;
                        	while (alias != null) {
                        		if (alias.getNodeType() == Node.ELEMENT_NODE) {
                        			String name2 = alias.getNodeName();
                                    String text2 = alias.getTextContent().trim();
                                    if (name2.contentEquals("alias")) {
                                    	aliases.add(text2);
                                    }
                        		}
                        		alias = alias.getNextSibling();
                        	}
                        }
                    } 
                    next = next.getNextSibling();
                }
                if (u.isInDirectory()) {
                    buildDialPatterns(u);
                }
                u.setAliases(aliases);
                m_userNameMap.put(u.getUserName(), u);
                
                // For each alias, add the user to the map
                for (String alias : aliases) {
					m_userNameMap.put(alias, u);
				}
                m_users.add(u);
            }

        } catch (Exception e) {
            LOG.fatal("Problem understanding document "+prop, e);
            System.exit(1);
        }
    }

    /**
     * Given a bunch of DTMF digits, return the list of users that matches
     * 
     * @param digits DTMF digits to match against user directory
     * @return a Vector of users that match
     */
    public Vector<User> lookupDTMF(String digits) {
        Vector<User> matches = new Vector<User>();
        for (User u : m_users) {
            if (u.isInDirectory() && u.getDialPatterns() != null) {
                for (String dialPattern : u.getDialPatterns()) {
                    if (dialPattern.startsWith(digits)) {
                        matches.add(u);
                        break;
                    }
                }
            }
        }
        return matches;
    }

    /**
     * See if a given user_name is valid (aka it can be dialed and reach a user)
     * 
     * @param userNname
     * 
     * @return user found or null
     */
    public User isValidUser(String userNname) {
        return m_userNameMap.get(userNname);
    }

    /**
     * Remove all non-letter characters, convert to upper case Remove diacritical marks if
     * possible
     * 
     * @param orig
     */
    static String compress(String orig) {
        if (orig == null) {
            return "";
        }

        String normal = orig.toUpperCase();

        // Brute force conversion of diacriticals
        normal = normal.replaceAll("[ÂÀÄÁÃ]", "A");
        normal = normal.replaceAll("[ÊÈËÉ]", "E");
        normal = normal.replaceAll("[ÎÌÏÍ]", "I");
        normal = normal.replaceAll("[ÔÒÖÓÕ]", "O");
        normal = normal.replaceAll("[ÛÙÜÚ]", "U");
        normal = normal.replaceAll("Ç", "C");
        normal = normal.replaceAll("Ñ", "N");

        // Remove non letters
        normal = normal.replaceAll("[^A-Z]", "");

        return normal;
    }

    /**
     * Map from letters to DTMF numbers
     */
    static String mapDTMF(String orig) {
        if (orig == null) {
            return "";
        }

        StringBuilder output = new StringBuilder(orig.length());
        for (int i = 0; i < orig.length(); i++) {
            String c = orig.substring(i, i + 1);
            int pos = s_letters.indexOf(c);
            if (pos >= 0) {
                // Output the corresponding position in numbers
                output.append(s_numbers.charAt(pos));
            }
        }
        return output.toString();
    }

    static String getUserName(String uri) {
        if (uri == null) {
            return null;
        }

        String userName = uri;
        int urlStart = userName.indexOf("sip:");

        if (urlStart >= 0) {
            userName = userName.substring(urlStart + 4);
        }

        int atStart = userName.indexOf("@");
        if (atStart <= 0) {
            return null;
        }

        userName = userName.substring(0, atStart);
        return userName.trim();
    }

    static String getDisplayName(String uri) {
        if (uri == null) {
            return null;
        }

        String displayName = "";
        int urlStart = uri.indexOf("sip:");

        if (urlStart < 0) {
            return null;
        }

        displayName = uri.substring(0, urlStart);
        displayName = displayName.trim();
        int gtStart = displayName.indexOf("<");
        if (gtStart >= 0) {
            displayName = displayName.substring(0, gtStart);
        }
        int quoteStart = displayName.indexOf('"');
        if (quoteStart >= 0) {
            displayName = displayName.substring(quoteStart + 1);
            int quoteEnd = displayName.indexOf('"');
            if (quoteEnd >= 0) {
                displayName = displayName.substring(0, quoteEnd);
            }
        }
        return displayName.trim();
    }

    /**
     * Parse the Display name into a list of DTMF sequences
     * 
     * Do one for Last name first
     * And one for First name first
     * @param u
     */
    static void buildDialPatterns(User u) {
        u.setDialPatterns(new Vector<String>());

        if (u.getDisplayName() == null) {
            return;
        }

        String[] names = u.getDisplayName().split("\\W");
        // Remove all non-character data, convert to upper case, convert to DTMF

        LinkedList<String> queue = new LinkedList<String>();
        for (String name : names) {
            String dtmf = mapDTMF(compress(name));
            if (dtmf.length() > 0) {
                queue.add(mapDTMF(compress(name)));
            }
        }

        // Given a b c d, generate:
        //    a b c d
        //    b c d a
        //    c d a b
        //    d a b c
        for (int i = 0; i < queue.size(); i++) {
            String mashup;
            String first = queue.poll(); // Pull first
            mashup = first;
            for (int j = 0; j < queue.size(); j++) {
                String next = queue.poll();
                mashup += next;
                queue.add(next);
            }
            queue.add(first); // Put first back (its now last)
            u.getDialPatterns().add(mashup);
        }
    }
}
