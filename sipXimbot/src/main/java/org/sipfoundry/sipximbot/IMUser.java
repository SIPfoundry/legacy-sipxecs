package org.sipfoundry.sipximbot;

import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.Socket;
import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collection;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.HashSet;
import java.util.List;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.apache.commons.httpclient.Credentials;
import org.apache.commons.httpclient.HttpClient;
import org.apache.commons.httpclient.UsernamePasswordCredentials;
import org.apache.commons.httpclient.auth.AuthPolicy;
import org.apache.commons.httpclient.auth.AuthScope;
import org.apache.commons.httpclient.methods.GetMethod;
import org.apache.log4j.Logger;
import org.jivesoftware.smack.Chat;
import org.jivesoftware.smack.MessageListener;
import org.jivesoftware.smack.RosterEntry;
import org.jivesoftware.smack.XMPPConnection;
import org.jivesoftware.smack.XMPPException;
import org.jivesoftware.smack.packet.Message;
import org.jivesoftware.smack.packet.Presence;
import org.sipfoundry.commons.freeswitch.ConferenceMember;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocket;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocketInterface;
import org.sipfoundry.commons.freeswitch.Set;
import org.sipfoundry.commons.userdb.ValidUsersXML;
import org.sipfoundry.sipximbot.CallHelper.CallHelperReturnCode;
import org.sipfoundry.sipximbot.IMContext.Command;
import org.sipfoundry.sipximbot.IMContext.Place;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public class IMUser {
        public enum UserPresence {
            UNKOWN,
            AVAILABLE,
            ONPHONE,
            INCONFERENCE,
            BUSY,    // corresponds to DND - do not disturb
            AWAY;
        } 
    
        private class DirUser {
            String m_fullName;
            String m_number;            
        }
        
        private Chat           m_chat;
        private String         m_resource;;
        private IMUser         m_callingUser;
        private IMContext      m_context;
        private FullUser       m_user;
        private XMPPConnection m_con;
        //private String         m_atNumber;
        //private Place          m_atPlace;
        private Date           m_atUntilTime;
        private Date           m_blockUntilTime;
        private String         m_callSubject;
        private Localizer      m_localizer;
        
        // collection of jabber ids to poke (let it be known) when this user becomes avaiable. 
        private Collection<IMUser>  m_usersToPoke;
        static final Logger LOG = Logger.getLogger("org.sipfoundry.sipximbot");
       
        
        IMUser(FullUser user, String jabberId, Presence presence, XMPPConnection con, Localizer localizer) {
            m_con = con;
            m_localizer = localizer;
            m_usersToPoke = new HashSet<IMUser>();
            m_user = user;      
            m_resource = null;
            // m_atPlace = Place.WORK;
   
            m_chat = m_con.getChatManager().createChat(jabberId, new MessageListener() {
                public void processMessage(Chat chat, Message message) {
                    if(message.getType() == Message.Type.error) {
                        // ignore error IMs
                        return;
                    }
                    
                    if(message.getBody() == null) {
                        return;
                    }
                                           
                    String from = chat.getParticipant();
                    if(from.indexOf('/') > 0) {
                        from = from.substring(0, from.indexOf('/'));
                    }       

                    
                    FullUser fromUser = IMBot.findUser(from);
                    
                    boolean deleteRosterEntry = fromUser == null;
                    if(fromUser != null) {
                        deleteRosterEntry  = !fromUser.getUserName().equals(m_user.getUserName());     
                    }
                    
                    if(deleteRosterEntry) {
                        // likely user has been deleted
                        m_chat.removeMessageListener(this);
                        RosterEntry entry = m_con.getRoster().getEntry(from);
                        try {
                            m_con.getRoster().removeEntry(entry);
                        } catch (XMPPException e) {

                        }                  
                        return;
                    }
                    
                    // update our view of the user
                    m_user = fromUser;
                    
                    LOG.debug("From " + from + " IM: " + message.getBody());
                    
                    synchronized(this) {
                        m_resource = message.getFrom();
                        m_context.setResource(m_resource);
                        boolean cmdComplete = m_context.receivedIM(message.getBody());
                        
                        if(cmdComplete) {
                            String cmdResult = ProcessCmd(message.getBody());
                            if (cmdResult.length() > 0) {
                                sendIM(cmdResult);
                            } 
    
                            m_context.clearContext();
                        }
                        m_resource = null;
                    }
                } 
            });  

            LOG.debug("Created chat for " + jabberId);
            m_context = new IMContext(m_chat, Command.NONE, m_localizer);
        }
        
        private void sendPokes() {
            for(IMUser imuser : m_usersToPoke) {
                imuser.sendIM(m_user.getDisplayName() + " " + localize("is_now_available"));
            }
            m_usersToPoke.clear();
        }
        
        public Date BlockCallsUntil() {
            Date now = new Date();
            if(m_blockUntilTime == null) {
                return null;
            }
                
            if (now.after(m_blockUntilTime)) {
                return null;
            } else {
                return m_blockUntilTime;       
            } 
        }
        
        // presence update sent from MyAssistant IM client
        public void setPresence(Presence presence) {
            Presence.Mode mode = presence.getMode();
            if(mode == null) 
                mode = Presence.Mode.available;
            
            if(mode == Presence.Mode.available) {
                sendPokes();
            }
        }
                       
        public void  addUsertoPoke(FullUser user) {
            IMUser imuser = IMBot.getIMUser(user);
            if(imuser != null) {
                m_usersToPoke.add(imuser);
            }
        }
        
        public String getCallSubject() {
            return m_callSubject;
        }
        
        public Chat getChat() {
            return m_chat;
        }
        
        public void setCallingIMUser(IMUser callingUser) {
            m_callingUser = callingUser;   
        }       
        
        public String currentlyAtUri(Place fromPlace, String fromNumber) {
            // see if we should be calling the user at a specific number
            String atUri = null;
            Date now = new Date();

            atUri = extensionToUrl(m_user.getUserName()); 

            boolean expired = false; 
            if(m_atUntilTime != null) {                           
                if(now.after(m_atUntilTime)) {
                    expired = true;
                }
            }    
            
            if(!expired) {
                if(fromPlace == Place.CELL) {
                    atUri = extensionToUrl(m_user.getCellNum());
                }
                
                if(fromPlace == Place.HOME) {
                    atUri = extensionToUrl(m_user.getHomeNum());
                }
                
                if(fromPlace == Place.UNKNOWN && fromNumber != null && fromNumber.length() > 0) {
                    atUri = extensionToUrl(fromNumber);    
                }
                
            }
            return atUri;
        }
        
        private String atName(Place fromPlace, String fromNumber) {
            Date now = new Date();
            String atName = null;
            
            boolean expired = false; 
            if(m_atUntilTime != null) {                           
                if(now.after(m_atUntilTime)) {
                    expired = true;
                }
            }    
                    
            if(!expired) {
                if(fromPlace == Place.CELL) {
                    atName = localize("cell");
                }
                
                if(fromPlace == Place.HOME) {
                    atName = localize("home");
                }
                
                if(fromPlace == Place.WORK) {
                    atName = localize("work");
                }
                
                if(fromPlace == Place.UNKNOWN && fromNumber != null) {
                    atName = fromNumber;
                }
            }    
            
            return atName;
        }
        
        
        private String localize(String prompt) {
            return m_localizer.localize(prompt);
        }
        
        private void Call(Place fromPlace, String fromNumber, String uriToCall, String nameToCall) {
            
            String fromURI = currentlyAtUri(fromPlace, fromNumber);
                                                    
            String atName = atName(fromPlace, fromNumber);
            if(atName != null) {
                atName = localize("from") + " " + atName;
            } else {
                atName = "";
            }
                    
            sendIM(localize("calling") + " " + nameToCall + " " + atName);                   

            try {    
                CallHelper callHelper = new CallHelper();

                CallHelperReturnCode ret = callHelper.call(fromURI, uriToCall, m_user.getUserName(), 20);
                switch (ret) {
                                
                case NO_ANSWER:
                    sendIM(localize("did_not_answer"));                    
                    break;
                    
                case BUSY:
                    sendIM(localize("phone_busy"));  
                    break;
                    
                case FORBIDDEN:
                    sendIM(localize("forbidden"));  
                    break;
                    
                case INVALID:                    
                    sendIM(localize("invalid"));                   
                    break;
                    
                case FAILURE:
                    sendIM(localize("failure"));  
                    break;                 
                }
                
            } catch (Exception e) {
                LOG.error("exception in Call " + e.getMessage());   
            }     
        }
        
        public void sendCommand(Command command, String msg) {
            m_context.sendCommand(command, msg);
        }       
         
        public void sendIM(String msg) {
            try {
                Message message = new Message();
                if(m_resource != null) {
                    message.setTo(m_resource);
                } else {
                    message.setTo(m_chat.getParticipant());
                }
                message.setBody(msg);
                m_chat.sendMessage(message);
            } catch (XMPPException e) {
                LOG.error("exception in sendIM " + e.getMessage());
            }
        }  
        
        private String walkDocCallHistory(Document doc, int daysOfHist) {
            
            NodeList matches = doc.getElementsByTagName("Row");    
            String histStr = "";  
            
            if(matches.getLength() > 0) {
          
                String caller = "";
                String callee = "";
                String startTime = "";
                String termination = "";
                String callTag = "";
                
                String callerUserPart;
                String callerDisplayPart;
                String line;
                String calleeIdent;
                int calleeIdentTail;
                String todayStr;
                DateFormat  inFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS zzzzzzzz");  
                DateFormat  fullFormat = new SimpleDateFormat("EEEE, MMMM d 'at' H:mm a");
                DateFormat  timeFormat = new SimpleDateFormat("'at' H:mm a");
                
                Date date;
                Calendar today = new GregorianCalendar();
                Calendar dateOfCall = new GregorianCalendar();
                today.setTime(new Date());                                         
                
                for (int userNum = 0; userNum < matches.getLength(); userNum++) {
                    Node match = matches.item(userNum);
                    Node next = match.getFirstChild();                
                
                    while (next != null) {
                        if (next.getNodeType() == Node.ELEMENT_NODE) {
                            if(next.getNodeName().trim().equals("caller_aor")) {
                                caller = next.getTextContent().trim();
                            }
                            
                            if(next.getNodeName().trim().equals("callee_aor")) {
                                callee = next.getTextContent().trim();
                            }
                            
                            if(next.getNodeName().trim().equals("start_time")) {
                                startTime = next.getTextContent().trim();
                            }
                            
                            if(next.getNodeName().trim().equals("termination")) {
                                termination = next.getTextContent().trim();
                            }   
                            
                            if(next.getNodeName().trim().equals("callee_route")) {
                                callTag = next.getTextContent().trim();
                            }        
                        }
                                        
                        next = next.getNextSibling();
                    }  
                    
                    callerUserPart = ValidUsersXML.getUserPart(caller);
                    callerDisplayPart = ValidUsersXML.getDisplayPart(caller);
                    
                    calleeIdent = callee.substring(callee.indexOf("sip:"));
                    calleeIdentTail = calleeIdent.indexOf(";");
                    if (calleeIdentTail == -1) {
                       calleeIdentTail = calleeIdent.length() - 1;
                    } 
                    calleeIdent = calleeIdent.substring(4, calleeIdentTail); 
                    boolean inbound = calleeIdent.equals(m_user.getIdentity());
                    
                    if(inbound && (termination.equals("A") | callTag.endsWith("VMR,VM"))) {
                        
                        try {
                            startTime += " GMT-0:00"; // time is GMT
                            date = inFormat.parse(startTime);
                                      
                            dateOfCall.setTime(date);
                            DateFormat df = fullFormat;
                            todayStr = "";
                            if(dateOfCall.get(Calendar.DAY_OF_YEAR) == today.get(Calendar.DAY_OF_YEAR)) {
                                if(dateOfCall.get(Calendar.YEAR) == today.get(Calendar.YEAR)) {
                                    df = timeFormat;
                                    todayStr = localize("today") + " ";
                                }
                            }
                        
                            line = callerDisplayPart + " (" + callerUserPart + "), " +
                                        todayStr + df.format(date) + ".  ";

                            line += UserPresenceAndStatus(callerUserPart, true);    
                            
                            if(histStr.length() == 0) {
                                if(daysOfHist == 1) {
                                    histStr = localize("missed_today");
                                } else {
                                    histStr = localize("missed_many_days") + " " + 
                                               daysOfHist + " " + localize("days") + " ..";
                                }    
                            }
      
                            histStr += "\n" + line;
                        } catch (ParseException e) {
                            LOG.error("exception in CallHistory Command " + e.getMessage());  
                        }
                    }
                }
            }      
            
            if(histStr.length() > 0) {
                
                sendIM(histStr);

                if(daysOfHist == 1) {
                    return localize("for_older_missed");
                } else {
                    return "";
                }
            } else {
                if(daysOfHist == 1) {
                    return localize("no_missed_today");
                } else {
                    return localize("no_missed_days") + " " + daysOfHist + " " + localize("days");
                }
            }
        }
        
        private String UserPresenceAndStatus(String userId, boolean includeName) {
            // lets see if this corresponds to an end user and if so 
            // determine the user's presence info
            String presAndStatus = "";
            FullUser user = FullUsers.update().isValidUser(userId);
            if(user != null) {
                UserPresence presence = IMBot.getUserPresence(user);
                String status = IMBot.getUserStatus(user);

                switch(presence) {
                    case AVAILABLE:
                        presAndStatus += " " + localize("is_available");    
                        break;
                    
                    case BUSY:
                        presAndStatus += " " + localize("is_busy");           
                        break;
                    
                    case AWAY:
                        presAndStatus += " " + localize("is_away");    
                        break;
                                 
                    case ONPHONE:
                        // if on the phone then OpenFire plugin will likely have
                        // set the status field
                        if(status != null) {
                            if (status.length() > 0) {
                                break;
                            }
                        }
                        presAndStatus += " " + localize("is_on_phone");    
                        break;                   
                        
                    case INCONFERENCE:
                        presAndStatus += " " + localize("is_conferenced");    
                        break;              
                }
                
                if(status != null) {
                    if(status.length() > 0) {
                        presAndStatus += " - " + status;
                    }
                }
                
                if(includeName && presAndStatus.length() > 0) {
                    presAndStatus = user.getDisplayName() + presAndStatus;
                }    
            }                     
 
            return presAndStatus;    
        }
        
        private DirUser walkDoc(Document doc, boolean callSearch) {
            
            DirUser dirUser = null;
            NodeList matches = doc.getElementsByTagName("entry");    
            
            if(matches.getLength() == 0) {
                sendIM(localize("no_matches"));
            } else if(matches.getLength() > 7) {
                sendIM(localize("too_many_matches"));               
            } else {            
 
                String firstName = "";
                String lastName = "";
                String number = "";
                
                for (int userNum = 0; userNum < matches.getLength(); userNum++) {
                    Node match = matches.item(userNum);
                    Node next = match.getFirstChild();                
                
                    while (next != null) {
                        if (next.getNodeType() == Node.ELEMENT_NODE) {
                            if(next.getNodeName().trim().equals("first-name")) {
                                firstName = next.getTextContent().trim();
                            }
                            
                            if(next.getNodeName().trim().equals("last-name")) {
                                lastName = next.getTextContent().trim();
                            }
                            
                            if(next.getNodeName().trim().equals("number")) {
                                number = next.getTextContent().trim();
                            }                                                       
                        }
                                        
                        next = next.getNextSibling();
                    }  
                    if(callSearch && matches.getLength() == 1) {
                        dirUser = new DirUser();
                        dirUser.m_fullName = firstName + " " + lastName;
                        dirUser.m_number = number;
                    } else {   
                        sendIM(firstName + " " + lastName + " (" + number + ") " + 
                               UserPresenceAndStatus(number, false));
                    }
                }
            }        
            return dirUser;
        }
        
        private DirUser findByName(String findTerm, boolean callSearch) {              
            
            RestfulRequest rr = new RestfulRequest(
                    ImbotConfiguration.get().getConfigUrl() + "/sipxconfig/rest/my/search", 
                    m_user.getUserName(), ImbotConfiguration.getSharedSecret());                   
            
            HttpURLConnection urlConn;
            try {
                urlConn = rr.getConnection("phonebook?query=" + findTerm);
          
                if (rr.get(urlConn)) {
                    InputStream in = urlConn.getInputStream();
                
                    DocumentBuilderFactory factory = DocumentBuilderFactory
                        .newInstance();
                    DocumentBuilder builder = factory.newDocumentBuilder();
                    Document doc = builder.parse(in);
                
                    return walkDoc(doc, callSearch);
                }
 
            } catch (Exception e) {
                LOG.error("exception in findByName " + e.getMessage());
            } 
            
            return null;            
        }
        
        private String getFromDate(int days) {

            DateFormat format = new SimpleDateFormat("yyyyMMdd");
            
            Date date = new Date();
            long millisec = date.getTime();
            long delta = (days-1)*24*60*60;
            delta *= 1000;
            
            millisec -= delta;
            date.setTime(millisec);            
            
            return format.format(date);
        }       
        
        private String callHistory() {

            int daysOfHist = m_context.getHistoryNum();
            
            RestfulRequest rr = new RestfulRequest(ImbotConfiguration.get().getcdrSecureUrl());
                         
            HttpURLConnection urlConn;
            try {
                urlConn = rr.getConnection(m_user.getUserName() + "?fromdate=" + getFromDate(daysOfHist));
            
                if (rr.get(urlConn)) {
                    InputStream in = urlConn.getInputStream();                    
                
                    DocumentBuilderFactory factory = DocumentBuilderFactory
                        .newInstance();
                    DocumentBuilder builder = factory.newDocumentBuilder();
                    Document doc = builder.parse(in);
                
                    return walkDocCallHistory(doc, daysOfHist);
                }
 
            } catch (Exception e) {
                LOG.error("exception in callHistory " + e.getMessage());
            } 
                             
            return localize("error");            
        }
        
        private String getUUID(FullUser user) {
            String uuid = null;
            HttpClient httpClient = new HttpClient();
            List<String> authPrefs = new ArrayList<String>(1);
            // call ivr API with digest auth policy
            authPrefs.add(AuthPolicy.DIGEST);
            httpClient.getParams().setParameter(AuthPolicy.AUTH_SCHEME_PRIORITY, authPrefs);
            Credentials credentials = new UsernamePasswordCredentials(m_user.getUserName(), ImbotConfiguration.getSharedSecret());
            httpClient.getState().setCredentials(new AuthScope(AuthScope.ANY_HOST, AuthScope.ANY_PORT, AuthScope.ANY_REALM),
                    credentials);
            GetMethod getUUID = new GetMethod(ImbotConfiguration.get().getVoicemailRootUrl() + "/mailbox/" + m_user.getUserName() + "/uuid");
            try {
                   int statusCode = httpClient.executeMethod(getUUID);
                   if (statusCode != 200) {
                       throw new Exception("failed to retrieve UUID");
                   }
                   InputStream in = getUUID.getResponseBodyAsStream();
                   DocumentBuilderFactory factory = DocumentBuilderFactory
                            .newInstance();
                   DocumentBuilder builder = factory.newDocumentBuilder();
                   Document doc = builder.parse(in);
                   
                   NodeList matches = doc.getElementsByTagName("uuid");    
                   
                   if(matches.getLength() != 0) {           
                       Node match = matches.item(0);
                       uuid = match.getTextContent().trim();
                       if(uuid.length() == 0) {
                           uuid = null;
                       }
                   }        
                    
     
            } catch (Exception e) {
                LOG.error("exception in getVoicemailRootUrl " + e.getMessage());
            } 
            
            return uuid;
        }
        
        private String doListen() {
            ImbotConfiguration config = ImbotConfiguration.get();
            
            FreeSwitchEventSocketInterface fses = new FreeSwitchEventSocket(config);           
            String result = "Ok";
            String uuid = getUUID(m_user);
            
            if(uuid == null) {
                result = localize("no_voice_msg");     
            } else {    
                try {                    
                    // TODO: for now assume localhost .. 
                    
                    Socket socket = new Socket("localhost", 8021);
                     
                    if (fses.connect(socket, "ClueCon")) {          
                        ListenIn listenIn = new ListenIn(fses, m_user.getIdentity(), 
                                                         uuid, config.getSipxchangeDomainName());
                        listenIn.go();                   
                    }
                    fses.close();
                } catch (Exception e) {
                    result = localize("error");   
                    LOG.error("exception in doListen " + e.getMessage());     
                }
            }
            return result;
        }
        
        private String doPickUp() {
            FreeSwitchEventSocketInterface fses = new FreeSwitchEventSocket(ImbotConfiguration.get());           
            String result = "Ok";
            String uuid = getUUID(m_user);
            
            if(uuid == null) {
                result = localize("no_voice_msg");     
            } else {    
                try {                    
                    // TODO: for now assume localhost .. 
                    Socket socket = new Socket("localhost", 8021);
                          
                    if (fses.connect(socket, "ClueCon")) {                                      
                        Set set = new Set(fses, uuid, "sipx_pickup", "yes");
                        set.go();                   
                    }
                    fses.close();
                } catch (Exception e) {
                    result = localize("error");   
                    LOG.error("exception in doPickUp " + e.getMessage());     
                }
            }
            return result;
        }
        
        private String extensionToUrl(String extension) {
            return extension + "@" + ImbotConfiguration.get().getSipxchangeDomainName();
        }

        private String ProcessCmd(String msg) {
            String cmdResult = "Ok";
            Collection<ConferenceMember> members;
            
            switch(m_context.getCommand()) {
            case REJECTCALL:
                // want to send IM back to calling party
                m_callingUser.sendIM(m_user.getDisplayName() + " says: " + msg);
                break;
                
            /*     
            case AT:                
            case AT_UNTIL:
                // update and store new location information (number, expire time)
                Place place = m_context.getPlace();
                if(place == Place.CELL && m_user.getCellNum() == null) {
                    cmdResult = "I don't know your cell number.";
                    break;
                }
                
                if(place == Place.HOME && m_user.getHomeNum() == null) {
                    cmdResult = "I don't know your home number.";
                    break;
                }
                
                m_atNumber = m_context.getPhoneNumber();
                m_atPlace = place;
                m_atUntilTime = m_context.getUntilTime();
                
                atName = atName();                
                if(atName != null) {
                    String untilTime = "";
                    if(m_atUntilTime != null) {          
                        DateFormat  timeFormat = new SimpleDateFormat("h:mm a zzz");
                        untilTime = " until " +  timeFormat.format(m_atUntilTime);     
                    }
                        
                    cmdResult = "The call command will call you at " + atName + untilTime + ".";
                }
                
                break;
                               
            case UNBLOCK:
                m_blockUntilTime = null;
                break;
                
            case STATUS:
                atName = atName();                
                if(atName != null) {
                    String untilTime = "";
                    if(m_atUntilTime != null) {       
                        DateFormat  timeFormat = new SimpleDateFormat("h:mm a zzz");
                        untilTime = " until " +  timeFormat.format(m_atUntilTime);                      
                    }
                        
                    sendIM("The call command will call you at " + atName + untilTime + ".");
                }
                
                Date blockCallsUntil = BlockCallsUntil();
                
                if(blockCallsUntil != null) {
                    DateFormat fmt = DateFormat.getTimeInstance(DateFormat.SHORT);                                 
                    sendIM("Blocking calls until " + fmt.format(blockCallsUntil) + ".");
                }                    
                
                cmdResult = "";
                break;
            */  
                
            case HISTORY:
                cmdResult = callHistory();
                break;
                
            case BLOCK_UNTIL:
                m_blockUntilTime = m_context.getUntilTime();
                break;
                              
            case CALL_FROM:
                
                String queryString = "";
                String numToCall = null;
                String nameToCall = "";
                Place fromPlace = Place.UNKNOWN;
                String fromNumber = null;
                
                cmdResult = "";
                
                if(m_context.getToPlace() == Place.CONFERENCE) {
                    numToCall = m_user.getConfNum();
                    if(numToCall == null) {
                        cmdResult = localize("conference_unknown");
                        break;
                    }
                    
                    String confPin = m_user.getConfPin();
                    if(confPin != null) {
                        queryString = "?confpin=" + confPin;
                    }
                    
                    nameToCall = localize("conference");
                }
                
                if(m_context.getToPlace() == Place.CELL) {
                    numToCall = m_user.getCellNum();
                    if(numToCall == null) {
                        cmdResult = localize("cell_unknown");
                        break;
                    }
                    nameToCall = localize("cell");
                }
                
                if(m_context.getToPlace() == Place.HOME) {
                    numToCall = m_user.getHomeNum();
                    if(numToCall == null) {
                        cmdResult = localize("home_unknown");
                        break;
                    }
                    nameToCall = localize("home");
                } 
                
                fromPlace = m_context.getFromPlace();
                
                if(m_context.getFromPlace() == Place.CELL) {
                    if(m_user.getCellNum() == null) {
                        cmdResult = localize("cell_unknown");
                        break;
                    }
                }
                
                if(m_context.getFromPlace() == Place.HOME) {
                    if(m_user.getHomeNum() == null) {
                        cmdResult = localize("home_unknown");
                        break;
                    }
                }
                
                if( fromPlace == Place.UNKNOWN) {
                    fromNumber = m_context.getFromPhoneNumber();
                }
                
                if(numToCall == null) {
                    numToCall = m_context.getToPhoneNumber();
                    // see if numeric
                    if(!numToCall.matches("[0-9[-.*#]]+")) {
                        DirUser dirUser = findByName(numToCall, true);
                        if(dirUser != null) {
                            numToCall = dirUser.m_number;
                            nameToCall = dirUser.m_fullName;
                        } else {
                            numToCall = null;
                        }
                    } else {
                        nameToCall = numToCall;
                    }                        
                }
                
                if(numToCall != null) {
                    Call(fromPlace, fromNumber, extensionToUrl(numToCall) + queryString, nameToCall);
                }                
                break;            
            
            case MUTE_CONF_PARTY:
            case DISC_CONF_PARTY:
            case UNMUTE_CONF_PARTY:
                String ConfCmd = null;
                String DisplayResult = null;
                
                switch(m_context.getCommand()) {
                    case MUTE_CONF_PARTY:
                        ConfCmd = "mute ";
                        DisplayResult = localize("muted");
                        break;
                        
                    case DISC_CONF_PARTY:
                        ConfCmd = "kick ";
                        DisplayResult = localize("disconnected");
                        break;
                        
                    case UNMUTE_CONF_PARTY:
                        ConfCmd = "unmute ";
                        DisplayResult = localize("unmuted");
                        break;
                }
                              
                String confParty = m_context.getConfParty();
                
                members = ConfTask.getMembers(m_user.getConfName());
                if(members == null) {
                    cmdResult = localize("conference_empty");
                    break;
                }
                
                cmdResult = null;
                
                for( ConferenceMember member : members) {
                    if(confParty != null) {
                        if(confParty.equals(member.memberIndex())) {  
                            cmdResult = ConfTask.ConfCommand(m_user, ConfCmd + member.memberId(), m_localizer);
                            if(cmdResult == null) {
                                cmdResult = member.memberName() + " (" + member.memberNumber() + ") " + DisplayResult;
                            }
                            break;
                        }    
                    } else {
                        cmdResult = ConfTask.ConfCommand(m_user, ConfCmd + member.memberId(), m_localizer);
                        
                        if(cmdResult != null) {
                            break;
                        }
                    }
                }
                
                if(cmdResult != null) {                 
                    break;
                }
                
                if(confParty == null) {
                    cmdResult = localize("all_participants") + " " + DisplayResult;
                } else {
                    cmdResult = localize("participant_not_exist");
                }
                break;
   
            case LOCK_CONF:
                cmdResult = ConfTask.ConfCommand(m_user, "lock", m_localizer);
                if(cmdResult == null) {
                    cmdResult = localize("conference_locked");
                }
                break;    
                
            case UNLOCK_CONF:
                cmdResult = ConfTask.ConfCommand(m_user, "unlock", m_localizer);
                if(cmdResult == null) {
                    cmdResult = localize("conference_unlocked");
                }
                break;    
                
            case WHO:
                String confName = m_user.getConfName();
                cmdResult = "";
                
                boolean isLocked = ConfTask.isLocked(confName);
                if(isLocked) {
                    sendIM(localize("conference_is_locked"));
                }
                
                members = ConfTask.getMembers(confName);
                                
                if(members != null) {
                    String muteStr;
                    for( ConferenceMember member : members) {
                        muteStr = "";
                        if(member.isMuted()) {
                            muteStr  = localize("is_muted");
                        }
                        
                        sendIM("[" + member.memberIndex() + "] " + member.memberName() +
                               " (" + member.memberNumber() + ") " + muteStr);
                    }
                } else {
                    cmdResult = localize("conference_empty");
                }               

                break; 
                
            case FIND:
                findByName(m_context.getFindTerm(), false);
                cmdResult = "";
                break;
                
            case PICKUP:
                cmdResult = doPickUp();
                break;
                
            case LISTENIN:
                cmdResult = doListen();
                break;                
            }    
            
            return cmdResult;
        }

    }   
    
    
