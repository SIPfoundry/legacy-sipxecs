/*
 *
 *
 * Copyright (C) 2008-2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipximbot;

import java.io.File;
import java.util.HashMap;
import java.util.Vector;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsersXML;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

/**
 * Holds the valid user data needed for the AutoAttendant, parsing from IMDB XML files
 * 
 */
public class FullUsers {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipximbot");
    private static File s_contactInfoFile = null;
    
    private static String s_contactInfoFileName = System.getProperty("conf.dir", "/etc/sipxpbx") + 
                                                                      "/contact-information.xml";

    private static long s_lastContactInfoModified = 0;
    
    private static Vector<FullUser> m_fullUsers;
    private static HashMap<String, FullUser> m_userNameMap;

    // hashmap in support of find jabber id to user lookup
    private static HashMap<String, FullUser> m_jidMap;
    
    // hashmap in support of confname to user lookup
    private static HashMap<String, FullUser> m_confMap;
    
    private static FullUsers s_fullUsers = null;
    private static ValidUsersXML s_validUserXML = null;

    /**
     * Private constructor for updatable singleton
     */
    private FullUsers() {
        m_userNameMap = new HashMap<String, FullUser>();
        m_jidMap = new HashMap<String, FullUser>();
        m_confMap = new HashMap<String, FullUser>();
        m_fullUsers = new Vector<FullUser>(); 
    }

    /**
     */
    public static synchronized FullUsers update() {
        ValidUsersXML newValidUserXML = null;
        boolean dataChanged = false;
        
        try {
            newValidUserXML = ValidUsersXML.update(null, true);
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        
        if(newValidUserXML != s_validUserXML) {
            dataChanged = true;
            s_validUserXML = newValidUserXML;
        }
        
        s_contactInfoFile = new File(s_contactInfoFileName);
        
        dataChanged |=  s_contactInfoFile.lastModified() != s_lastContactInfoModified;
        
        if (dataChanged) {
            s_fullUsers = new FullUsers();           
            s_fullUsers.loadInfo();
        }
        
        return s_fullUsers;
    }
    
    public static Vector<FullUser> GetUsers() {
        return(m_fullUsers);
    }

    
    /**
     * Load the contact-information.xml file
     */
    void loadInfo() {
        LOG.info("Loading contact-information.xml configuration");
     
        for (User u : s_validUserXML.GetUsers()) {
            
            FullUser fullUser = new FullUser(u);
            
            m_fullUsers.add(fullUser);
            m_userNameMap.put(fullUser.getUserName(), fullUser);
        }
        
        Document contactInfoDoc = null;
        
        try {          
            s_lastContactInfoModified = s_contactInfoFile.lastModified();

            DocumentBuilder builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
            contactInfoDoc = builder.parse(s_contactInfoFile);
            
        } catch (Throwable t) {
            LOG.fatal("Something went wrong loading the validusers.xml file.", t);
            System.exit(1);
        }
        
        walkContactInfoDoc(contactInfoDoc);
    }
    
    private void walkContactInfoDoc(Document doc) {
        
        NodeList users = doc.getElementsByTagName("user");    
        FullUser user;
        ImbotConfiguration config = ImbotConfiguration.get();
                             
        for (int userNum = 0; userNum < users.getLength(); userNum++) {
            Node match = users.item(userNum);
            Node next = match.getFirstChild();       
                        
            user = null;

            while (next != null) {
                if (next.getNodeType() == Node.ELEMENT_NODE) {
                    if(next.getNodeName().trim().equals("userName")) {
                        user = m_userNameMap.get(next.getTextContent().trim());
                    }                        
                        
                    if(user != null) {
                        if(next.getNodeName().trim().equals("conferences")) {
                            Node conf = next.getFirstChild();
                            if(conf!= null) {
                                conf = conf.getNextSibling().getFirstChild();
                                
                                while(conf != null) {
                                    if(conf.getNodeName().trim().equals("name")) {
                                        user.setConfName(conf.getTextContent().trim());
                                        m_confMap.put(user.getConfName(), user);
                                    }
                            
                                    if(conf.getNodeName().trim().equals("extension")) {
                                        user.setConfNum(conf.getTextContent().trim());
                                    }
                                    
                                    if(conf.getNodeName().trim().equals("pin")) {
                                        user.setConfPin(conf.getTextContent().trim());
                                    }
                                    
                                    conf = conf.getNextSibling();
                                }
                            }
                        }
                                                                                       
                        if(next.getNodeName().trim().equals("cellPhoneNumber")) {
                            user.setCellNum(next.getTextContent().trim());
                        }
                            
                        if(next.getNodeName().trim().equals("cellPhoneNumber")) {
                            user.setCellNum(next.getTextContent().trim());
                        }
                        
                        if(next.getNodeName().trim().equals("homePhoneNumber")) {
                            user.setHomeNum(next.getTextContent().trim());
                        }
                        
                        if(next.getNodeName().trim().equals("conferenceEntryIM")) {
                            user.setConfEntryIM(next.getTextContent().trim());
                        }
                
                        if(next.getNodeName().trim().equals("conferenceExitIM")) {
                            user.setConfExitIM(next.getTextContent().trim());
                        }
                        
                        if(next.getNodeName().trim().equals("leaveMsgBeginIM")) {
                            user.setVMEntryIM(next.getTextContent().trim());
                        }
                        
                        if(next.getNodeName().trim().equals("leaveMsgEndIM")) {
                            user.setVMExitIM(next.getTextContent().trim());
                        }
                        
                        if(next.getNodeName().trim().equals("imId")) {
                            user.setjid(next.getTextContent().trim() + "@" + config.getSipxchangeDomainName());
                            m_jidMap.put(user.getjid(), user);
                        }                                         
                        
                        if(next.getNodeName().trim().equals("alternateImId")) {
                            user.setAltjid(next.getTextContent().trim());
                            m_jidMap.put(user.getAltjid(), user);
                        }    
                    }                     
                }  
                next = next.getNextSibling();
            }
        }        
    }
    
    /**
     * See if a given user_name is valid (aka it can be dialed and reach a user)
     * 
     * @param userNname
     * 
     * @return user found or null
     */
    public FullUser isValidUser(String userNname) {
        return m_userNameMap.get(userNname);
    }

    public FullUser findByjid(String jid) {
        return m_jidMap.get(jid);
    }
    
    public FullUser findByConfName(String confName) {
        return m_confMap.get(confName);
    }
    

}
