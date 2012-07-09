/*
 *
 *
 * Copyright (C) 2008-2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.userdb;

import java.io.File;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.Vector;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.userdb.User.EmailFormats;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

/**
 * Holds the valid user data needed for the AutoAttendant, parsing from IMDB XML files
 * 
 */
public class ValidUsersXML {
    private static Logger LOG = Logger.getLogger(ValidUsersXML.class);
    private static ValidUsersXML s_current;
    private static File s_validUsersFile;
    private static long s_lastModified;

    // Mapping of letters to DTMF numbers.
    // Position of letter in letters maps to corresponding position in numbers
    private static String s_letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    private static String s_numbers = "22233344455566677778889999";

    private static Vector<User> m_users;
    private static HashMap<String, User> m_userNameMap;
    private static String s_validUsersFileName = System.getProperty("conf.dir", "/etc/sipxpbx")+"/validusers.xml";


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
    public static ValidUsersXML update(Logger log, boolean load) throws Exception {
        if (log != null) {
            LOG = log;
        }
        if (s_current == null || s_validUsersFile == null ||
                s_validUsersFile.lastModified() != s_lastModified) {
            s_current = new ValidUsersXML();
            if (load) {
                s_current.loadXML();
            }
        }
        return s_current;
    }
    
    public static void setValidUsersFileName(String fileName ) {
        s_validUsersFileName = fileName;
        s_validUsersFile = null;
    }
    
    
    public static Vector<User> GetUsers() {
        return(m_users);
    }

    /**
     * Load the validusers.xml file
     * 
     * Looks in directory specified by JVM argument -Dconf.dir.  If -Dconf.dir isn't there, it 
     * uses /etc/sipxpbx.
     * 
     * @throws Exception on trouble reading or parsing the file
     */
    void loadXML() throws Exception {
        LOG.info("Loading validusers.xml configuration");
        Document validUsers = null; 

        try {
            s_validUsersFile = new File(s_validUsersFileName);
            s_lastModified = s_validUsersFile.lastModified();
            DocumentBuilder builder;
            builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
            validUsers = builder.parse(s_validUsersFile);
        } catch (Exception e) {
            LOG.error(String.format("Something went wrong loading the %s file.", s_validUsersFile.getPath()), e);
            throw(e);
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
                HashMap<String, DistributionList> distributionLists = new HashMap<String, DistributionList>();
                int emailCount = 0;
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
                        } else if ( name.contentEquals("passtoken")) {
                            u.setPasstoken(text);
                        } else if (name.contentEquals("pintoken")) {         
                            u.setPintoken(text);
                        } else if (name.contentEquals("inDirectory")) {
                            u.setInDirectory(Boolean.parseBoolean(text));
                        } else if (name.contentEquals("hasVoicemail")) {
                            u.setHasVoicemail(Boolean.parseBoolean(text));
                        } else if (name.contentEquals("userBusyPrompt")) {
                            u.setUserBusyPrompt(Boolean.parseBoolean(text));
                        } else if (name.contentEquals("voicemailTui")) {         
                            u.setVoicemailTui(text);
                        } else if (name.contentEquals("canRecordPrompts")) {
                            u.setCanRecordPrompts(Boolean.parseBoolean(text));
                        } else if (name.contentEquals("canTuiChangePin")) {
                            u.setCanTuiChangePin(Boolean.parseBoolean(text));
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
                        } else if (name.contentEquals("distributions")) {
                            loadDistributionLists(distributionLists, next);
                        } else if (name.contentEquals("email")) {
                            Node emailChild = next.getFirstChild();
                            emailCount++;
                            while(emailChild != null) {
                                if (emailChild.getNodeType() == Node.ELEMENT_NODE) {
                                    String name2 = emailChild.getNodeName();
                                    String text2 = emailChild.getTextContent().trim();
                                    if (name2.contentEquals("address")) {
                                        if (emailCount == 1) {
                                            u.setEmailAddress(text2);
                                        } else {
                                            u.setAltEmailAddress(text2);
                                        }
                                    } else if (name2.contentEquals("notification")) {
                                        String attachAudio = getAttribute(emailChild, "attachAudio", "false");
                                        if (emailCount == 1) {
                                            u.setEmailFormat(text2);
                                            u.setAttachAudioToEmail(Boolean.parseBoolean(attachAudio));
                                        } else {
                                            u.setAltEmailFormat(text2);
                                            u.setAltAttachAudioToEmail(Boolean.parseBoolean(attachAudio));
                                        }
                                    } else if (name2.contentEquals("imap")) {
                                        loadImapInfo(u, emailChild);
                                    }
                                }
                                emailChild = emailChild.getNextSibling();
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
                if (distributionLists.size() > 0) {
                    u.setDistributionLists(distributionLists);
                }
                m_users.add(u);
            }

        } catch (Exception e) {
            LOG.fatal("Problem understanding document "+prop, e);
            System.exit(1);
        }
    }

    private String getAttribute(Node element, String attribute, String missingValue) {
        String value = missingValue;
        Node attNode = element.getAttributes().getNamedItem(attribute);
        if (attNode != null) {
            value = attNode.getNodeValue();
        }
        return value;
    }

    private void loadImapInfo(User u, Node imapNode) {
        Node imapChild = imapNode.getFirstChild();
        ImapInfo i = new ImapInfo();
        i.setSynchronize(Boolean.parseBoolean(getAttribute(imapNode, "synchronize", "false")));
        while (imapChild != null) {
            if (imapChild.getNodeType() == Node.ELEMENT_NODE) {
                String name2 = imapChild.getNodeName();
                String text2 = imapChild.getTextContent().trim();
                if (name2.contentEquals("host")) {
                    i.setHost(text2);
                } else if (name2.contentEquals("port")) {
                    i.setPort(text2);
                } else if (name2.contentEquals("useTLS")) {
                    i.setUseTLS(Boolean.parseBoolean(text2));
                } else if (name2.contentEquals("account")) {
                    i.setAccount(text2);
                } else if (name2.contentEquals("password")) {
                    i.setPassword(text2);
                }
            }
            imapChild = imapChild.getNextSibling();
        }
        // If account isn't set, use the e-mail username
        if (i.getAccount() == null || i.getAccount().length() == 0) {
            if(u.getEmailAddress() != null) {
                i.setAccount(u.getEmailAddress().split("@")[0]); 
            }
        }
        u.setImapInfo(i);
        if(i.isSynchronize()) {
            u.setEmailFormat(EmailFormats.FORMAT_IMAP);
            u.setAttachAudioToEmail(true);
        }
    }
    
    private void loadDistributionLists(HashMap<String, DistributionList> distributionLists, Node distributionNode) {
        Node lists = distributionNode.getFirstChild() ;
        while (lists != null) {
            if (lists.getNodeType() == Node.ELEMENT_NODE) {
                String name2 = lists.getNodeName();
                if (name2.contentEquals("list")) {
                    String digits = getAttribute(lists, "digits", null);
                    String id = getAttribute(lists, "id", null);;
                    String audio = getAttribute(lists, "audio", null);
                    DistributionList dl = new DistributionList(id, audio);
                    Node entry = lists.getFirstChild();
                    while (entry != null) {
                        if (entry.getNodeType() == Node.ELEMENT_NODE) {
                            String name3 = entry.getNodeName();
                            String text3 = entry.getTextContent().trim();
                            if (name3.contentEquals("mailbox")) {
                                dl.addMailboxString(text3);
                            } else if (name3.contentEquals("systemlist")) {
                                dl.addSystemListString(text3);
                            }
                        }
                        entry = entry.getNextSibling();
                    }
                    distributionLists.put(digits, dl);
                }
            }
            lists = lists.getNextSibling();
        }
    }
    
    /**
     * Given a bunch of DTMF digits, return the list of users that matches
     * 
     * @param digits DTMF digits to match against user directory
     * @param onlyVoicemailUsers limit match to users in directory who have voicemail
     * @return a Vector of users that match
     */
    public Vector<User> lookupDTMF(String digits, boolean onlyVoicemailUsers) {
        Vector<User> matches = new Vector<User>();
        for (User u : m_users) {
            if (u.isInDirectory() && u.getDialPatterns() != null) {
                for (String dialPattern : u.getDialPatterns()) {
                    if (dialPattern.startsWith(digits)) {
                        if (!onlyVoicemailUsers || u.hasVoicemail()) {
                            matches.add(u);
                            break;
                        }
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
    public User getUser(String userNname) {
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

    public static String getDomainPart(String uri) {
        if (uri == null) {
            return null;
        }

        String domainName = uri;
        int atStart = domainName.indexOf("@");
        if (atStart <= 0) {
            return null;
        }

        domainName = domainName.substring(atStart+1);
        return domainName.trim();
    }

    public static String getUserPart(String uri) {
        if (uri == null) {
            return null;
        }

        String userName = uri;
        int urlStart = userName.indexOf("sip:");

        if (urlStart >= 0) {
            userName = userName.substring(urlStart + 4);
        } else {
            int gtStart = userName.indexOf("<");
            if (gtStart >= 0) {
                userName = userName.substring(gtStart+1);
            }
        }

        int atStart = userName.indexOf("@");
        if (atStart <= 0) {
            return null;
        }

        userName = userName.substring(0, atStart);
        return userName.trim();
    }

    public static String getDisplayPart(String uri) {
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
