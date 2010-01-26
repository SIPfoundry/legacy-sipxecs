package org.sipfoundry.sipximbot;

import java.net.MalformedURLException;
import java.net.URL;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import org.apache.log4j.Logger;
import org.jivesoftware.smack.ConnectionConfiguration;
import org.jivesoftware.smack.PacketListener;
import org.jivesoftware.smack.Roster;
import org.jivesoftware.smack.RosterEntry;
import org.jivesoftware.smack.RosterListener;
import org.jivesoftware.smack.XMPPConnection;
import org.jivesoftware.smack.XMPPException;
import org.jivesoftware.smack.filter.PacketFilter;
import org.jivesoftware.smack.filter.PacketTypeFilter;
import org.jivesoftware.smack.packet.Packet;
import org.jivesoftware.smack.packet.PacketExtension;
import org.jivesoftware.smack.packet.Presence;
import org.jivesoftware.smack.util.StringUtils;
import org.jivesoftware.smackx.packet.VCard;
import org.sipfoundry.sipximbot.IMUser.UserPresence;

public class IMBot {
    private static Roster m_roster;
    private static FullUsers m_fullUsers;
    
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipximbot");
            
    // key is Jabber Id, value is IMUser
    private static Map<String, IMUser> m_ChatsMap = Collections
            .synchronizedMap(new HashMap<String, IMUser>());
    
    private static class IMClientThread extends Thread {
        private static XMPPConnection m_con;
         
        public IMClientThread() {
        }
        
        /* IMbot client computes the SHA1 hash of the avatar image data itself. 
         * Include this hash in the user's presence information 
         * as the XML character data of the <photo/> child of an <x/> element 
         * qualified by the 'vcard-temp:x:update' namespace
         */
        private class AvatarUpdateExtension implements PacketExtension {
            private String photoHash;

            public void setPhotoHash(String hash) {
                photoHash = hash;
            }

            public String getElementName() {
                return "x";
            }

            public String getNamespace() {
                return "vcard-temp:x:update";
            }

            public String toXML() {
                StringBuffer buf = new StringBuffer();
                buf.append("<").append(getElementName()).append(" xmlns=\"").append(getNamespace()).append("\">");
                buf.append("<photo>");
                buf.append(photoHash);
                buf.append("</photo>");
                buf.append("</").append(getElementName()).append(">");
                return buf.toString();
            }
        }
        
        private void updateAvatar(ImbotConfiguration config) {
                
            boolean savingAvatar = false;
            VCard vCard = new VCard();
            try {
                vCard.load(m_con);
                URL url;
                try {
                    url = new URL("file://" + System.getProperty("var.dir", "/var/sipxdata") + "/sipximbot/image/avatar.jpg");
                    vCard.setAvatar(url);
                    savingAvatar = true;
                    vCard.save(m_con);
                    updateAvatarPresence(vCard);
                } catch (MalformedURLException e) {
                    LOG.error("Malformed URL in updateAvatar ");     
                }    
 
            } catch (XMPPException xmppe) {
                if(savingAvatar) {
                    // google talk doesn't like us saving avatars immediately after login!
                    try {
                        sleep(10000);
                    } catch (InterruptedException e) {

                    }
                    try {
                        vCard.save(m_con);
                        updateAvatarPresence(vCard);
                    } catch (XMPPException e) {
                        LOG.error("could not update Avatar " + e.getMessage());     
                    }
                }        
            }           
        }
        
        private void updateAvatarPresence(VCard vCard) {
            Presence aPresence = new Presence(Presence.Type.available);
            AvatarUpdateExtension AvatarExt = new AvatarUpdateExtension();
            AvatarExt.setPhotoHash(vCard.getAvatarHash());
            aPresence.addExtension(AvatarExt);
            aPresence.setStatus("Type help for a list of commands.");
            aPresence.setFrom(ImbotConfiguration.get().getMyAsstAcct());
            m_con.sendPacket(aPresence);
        }
        
        
        private boolean connectToXMPPServer() {
            
            ImbotConfiguration config;
            ConnectionConfiguration conf;
            
            for(;;) {
                try {
                    config = ImbotConfiguration.get();
                    conf = new ConnectionConfiguration(config.getOpenfireHost(), 5222);
                    conf.setSASLAuthenticationEnabled(false); // disable SASL to cope with cases where XMPP domain != FQDN (XX-7293)
                    Roster.setDefaultSubscriptionMode(Roster.SubscriptionMode.manual);
                    m_con = new XMPPConnection(conf);   
                    m_con.connect();
                    
                    String username = config.getMyAsstAcct().split("@")[0]; // only keep user part and ditch the @domain part if present
                    m_con.login(username, config.getMyAsstPswd());
                    return true;  
                } catch (Exception e) {
                    // typically get this exception if server is unreachable or login info is wrong
                    // only thing do it is periodically retry just like any other IM client would

                    LOG.error("Could not login to XMPP server " + e.getMessage());     
                }
                try {
                    sleep(5000);
                } catch (InterruptedException e) {
                    return false;
                }
            }
        }
        
        public static void AddToRoster(FullUser user) {         
            Presence presPacket = new Presence(Presence.Type.subscribe); 
            presPacket.setFrom(ImbotConfiguration.get().getMyAsstAcct());
            
            if(user.getjid() != null) {  
                presPacket.setTo(user.getjid());                        
                m_con.sendPacket(presPacket);
            }

            if(user.getAltjid() != null) {  
                presPacket.setTo(user.getAltjid());                                    
                m_con.sendPacket(presPacket);
            }
        }
        
        public void run() {           
            
            boolean running = true;
                               
            if(!connectToXMPPServer()) {
                return;
            }

            updateAvatar(ImbotConfiguration.get());
            m_roster = m_con.getRoster();
                                  
            class PresenceListener implements PacketListener {
                
                public void processPacket(Packet packet) {
                    Presence presence = (Presence) packet;
                    Presence presPacket = null;
             
                    switch (presence.getType()) {
                    case subscribe:
                        String jid = presence.getFrom();     
                        if(jid.indexOf('/') > 0) {
                            jid = jid.substring(0, jid.indexOf('/'));
                        }                   
                        
                        FullUser user = findUser(jid);
                        if(user == null) {
                            LOG.error("Rejected subscription from " + jid);     
                            presPacket = new Presence(Presence.Type.unsubscribed); 
                        } else {
                            LOG.info("Accepted subscription from " + jid); 
                            presPacket = new Presence(Presence.Type.subscribed); 
                        }                        
                        presPacket.setTo(presence.getFrom());
                        presPacket.setFrom(presence.getTo()); 
                        
                        m_con.sendPacket(presPacket);
                        
                        try {
                            sleep(1000);
                        } catch (InterruptedException e) {
                        }
                        
                        if((user != null) && (m_ChatsMap.get(jid) == null))  {
                            // now subscribe the sender's presence
                            presPacket.setType(Presence.Type.subscribe);
                           
                            m_con.sendPacket(presPacket);
                            
                            IMUser imuser = new IMUser(user, jid, null, m_con);
                            m_ChatsMap.put(jid, imuser);
                        }
                        
                        break;
                    case unsubscribe:
                        LOG.error("Received unexpected unsubscribe for " + presence.getFrom());     
                        break;
                    }
                }
            }
             
            PacketFilter filter = new PacketTypeFilter(Presence.class);
            m_con.addPacketListener(new PresenceListener(), filter);                       
            
            // create map with initial presence and status info
            
            FullUser user;
            Collection<RosterEntry> entries = m_roster.getEntries();
            for (RosterEntry entry : entries) {
                user = findUser(entry.getUser());
                if(user != null) {
                    IMUser imuser = new IMUser(user, entry.getUser(), m_roster.getPresence(entry.getUser()), m_con);                               
                    m_ChatsMap.put(entry.getUser(), imuser);
                } else {
                    try {
                        m_roster.removeEntry(entry);
                        LOG.error("Removing old roster entry " + entry.getUser());
                    } catch (XMPPException e) {
                        LOG.error("Could not remove roster entry " + entry.getUser());     
                    }
                }
            }            

            m_roster.addRosterListener(new RosterListener() {

                public void entriesAdded(Collection<String> entries) {
                    for(String address : entries) {
                        if(m_ChatsMap.get(address) != null) {
                            // already in chat map, this will happen
                            // if subscription was successful
                            continue;
                        }          
                        
                        FullUser user = findUser(address);
                        if(user == null) {
                            LOG.error("Rejected addition from " + address);
                        } else {                        
                            IMUser imuser = new IMUser(user, address, null, m_con);
                            m_ChatsMap.put(address, imuser);
                            LOG.debug("Entry added: " + address);
                        }                                                             
                    }
                }

                public void entriesDeleted(Collection<String> addresses) {
                    // Contacts have been removed from the roster
                    for(String address : addresses) {
                        for (RosterEntry entry : m_roster.getEntries()) {
                            if(address.equals(entry.getUser())) {
                                LOG.debug("Removing from roster: " + entry.getUser());
                                m_ChatsMap.remove(entry.getUser());
                            }    
                        }
                    }
                }
                
                public void entriesUpdated(Collection<String> arg0) {                
                }

                public void presenceChanged(Presence presence) {
                    
                    String from = presence.getFrom();
                    if(from.indexOf('/') > 0) {
                        from = from.substring(0, from.indexOf('/'));
                    }                   
                    m_ChatsMap.get(from).setPresence(presence);
                }
            });
            
            while (running) {
                try {
                    sleep(5000);
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                    running = false;
                }
            }
            m_con.disconnect();
        }
    }  
    
    static public synchronized FullUser findUser(String jid) {        
        m_fullUsers = FullUsers.update();       
        return(m_fullUsers.findByjid(jid));
    }
    
    static private String getjid(FullUser user) {
        String jid = user.getjid();
        if(jid == null) {
            jid = user.getAltjid();
        }    
        return jid;
    }
    
    static public String getUserStatus(FullUser user) {
        // return status corresponding to primary IM Id. If not filled in
        // try altId and if not filled in either then assume AVAILABLE
        String jid = getjid(user);
        if(jid == null) {
            return null;
        } else { 
            Presence pres = m_roster.getPresence(jid);
            
            if(pres != null) {
                return pres.getStatus();
            } else {
                return null;
            }       
        }
    }
    
    static public UserPresence getUserPresence(FullUser user) {
        // return presence corresponding to primary IM Id. If not filled in
        // try altId and if not filled in either then assume AVAILABLE
     
        if(ConfTask.inConferenceSince(user.getUserName()) != null) {
            return UserPresence.INCONFERENCE;
        }
        
        PhonePresence phonePresence;
        try {
            phonePresence = new PhonePresence();
            phonePresence.isUserOnThePhone(user.getUserName());
            if(phonePresence.isUserOnThePhone(user.getUserName())) {
                return UserPresence.ONPHONE;
            }    
        } catch (Exception e) {

        }
        
        String jid = getjid(user);
        if(jid == null) {
            return UserPresence.AVAILABLE;
        } else {            
            Presence pres = m_roster.getPresence(jid);

            if (pres == null)
                return UserPresence.UNKOWN;
            
            if(pres.getType() == Presence.Type.unavailable)
                return UserPresence.UNKOWN;
                
            Presence.Mode mode = pres.getMode();
            if(mode == null) 
                return UserPresence.AVAILABLE;           
                              
            if(mode == Presence.Mode.away) {
                return UserPresence.AWAY;
            }
            
            if(mode == Presence.Mode.chat) 
                return UserPresence.AVAILABLE;
            
            if(mode == Presence.Mode.dnd) 
                return UserPresence.BUSY;
            
            if(mode == Presence.Mode.xa) 
                return UserPresence.AWAY;
          
            return UserPresence.UNKOWN;                
                     
        }
    }
     
    // send to both user's IM accounts 
    public static void sendIM(String userName, String msg) {
        m_fullUsers = FullUsers.update();
        FullUser user = m_fullUsers.isValidUser(userName);
        sendIM(user, msg);    
    }    
    
    public static void sendIM(FullUser user, String msg) {        
        if(user != null) {
            IMUser toIMUser;
            
            String jid = user.getjid();
            if(jid != null) {
                toIMUser = m_ChatsMap.get(jid);  
                if(toIMUser != null) {
                    toIMUser.sendIM(msg);
                }
            }    

            jid = user.getAltjid();
            if(jid != null) {
                toIMUser = m_ChatsMap.get(jid);
                if(toIMUser != null) {
                    toIMUser.sendIM(msg);
                }
            }                
        }         
    }
    
    static public IMUser getIMUser(FullUser user) {
        String jid = getjid(user);
        if(jid == null) {
            return null;
        }
        return m_ChatsMap.get(jid);
    }
    
    static public void SendReturnCallIM(FullUser toUser, FullUser fromUser, 
                                       String callingName, String callingNumber) {

        IMUser toIMuser = getIMUser(toUser);
        IMUser fromIMuser = getIMUser(fromUser);
        
        if(toIMuser != null && fromIMuser != null) {
            toIMuser.setCallingIMUser(fromIMuser);
        
            toIMuser.sendIM(callingName + " (" + callingNumber +  
                            ") called and would like you to call back");
        }
    }
    
    public static void AddToRoster(FullUser user) {
        IMClientThread.AddToRoster(user);
    }

    static public void init() {
                      
        IMClientThread imThread = new IMClientThread();
        imThread.start();          
    }
}           

    
    
