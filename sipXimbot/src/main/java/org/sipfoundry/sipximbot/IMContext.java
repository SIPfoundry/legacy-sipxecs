package org.sipfoundry.sipximbot;

import java.util.Date;
import java.util.StringTokenizer;
import org.apache.log4j.Logger;
import org.jivesoftware.smack.Chat;
import org.jivesoftware.smack.XMPPException;
import org.jivesoftware.smack.packet.Message;
import org.sipfoundry.commons.freeswitch.Localization;

/*
 *  this class is responsible with conversing with the user until a command has
 *  been discerned. Commands can start with the user sending myassistant an IM or
 *  can start with myassistant sending an IM (eg screen pop). Who started the conversation
 *  is actually of no importance 
 *  
 *  items that must be stored on a per user basis
 *  - block list (web portal only?)
 *  - curent location *(at command), until time (long)
 *  - accept/block calls, until time (long)
 *  - announce calls, none, IM, telephone (web portal only)?
 *  
 *  When a context "completes" (received IM returns true) it retains the following:
 *    - none
 *    - call <name or number string> <subject>
 *    - conference <two or more names/numbers> <subject>
 *    - at <phone number string>, <until-time time long>
 *    - block-calls <until-time long>
 *    - accept calls
 *    - status
 *    - history <number>
 *    - help
 *    - reject call <reason string>
 *  
 */

public class IMContext {
    
    // class represents the necessary detail of the IM conversion to know 
    // how to respond to an IM received from the user
    
    public enum Place {
        CELL,
        HOME,
        WORK,
        CONFERENCE,    // only meaning full as a target place .. ie "place me in my conference"
        UNKNOWN
    }
    
    public enum Command {
        NONE,
        CALL,
        CALL_FROM,
        FIND,
        AT,    
        AT_UNTIL,
        BLOCK,
        BLOCK_UNTIL,
        UNBLOCK,
        HISTORY,
        HELP,
        STATUS,
        REJECTCALL,
        MUTE_CONF_PARTY,
        UNMUTE_CONF_PARTY,
        DISC_CONF_PARTY, 
        LOCK_CONF,
        UNLOCK_CONF,
        WHO,
        CONFERENCE,
        PICKUP,
        LISTENIN 
    }
    
    private Chat    m_chat;
    private String  m_resource;
    private Command m_command;
    private Date    m_untilTime;
    
    private String  m_fromPhoneNumber;
    private Place   m_fromPlace;
    
    private String  m_toPhoneNumber;
    private Place   m_toPlace;  
    
    private String  m_confParty;
    private boolean m_complete; 
    private String  m_findTerm;
    
    // m_parsedUntil is true if we are in the middle of a block or at command and have parsed the word 
    // "until" but are waiting on the user to enter a time 
    private int     m_historyNum;
    private long    m_timeReceived;
    private Localizer m_localizer;
    
    private static final long s_TIMEOUT = 15*1000; // ten second user response timeout 
    
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipximbot");
        
    public IMContext(Chat chat, Command command, Localizer localizer) {
        clearContext();
        m_command = command;
        m_chat = chat;       
        m_localizer = localizer;
    }
    
    public void clearContext() {
        m_timeReceived = System.currentTimeMillis();
        m_complete = false;
        m_command = Command.NONE;
        m_fromPlace = Place.UNKNOWN;
        m_toPlace = Place.UNKNOWN;
        m_untilTime = null;
        m_fromPhoneNumber = null;
        m_toPhoneNumber = null;
        m_confParty = null;
    }
    
    public void setChat(Chat chat) {
        m_chat = chat;
    } 
    
    public Place getFromPlace() {
        return m_fromPlace;
    }

    public String getFromPhoneNumber() {
        return(m_fromPhoneNumber);
    }   
    
    public Place getToPlace() {
        return m_toPlace;
    }

    public String getToPhoneNumber() {
        return(m_toPhoneNumber);
    }   
    
    public Command getCommand() {
        if(m_complete) {
            return m_command;
        } else {
            return Command.NONE;
        }
    }
    
    public void sendCommand(Command command, String msg) {
        m_command = command;
        m_timeReceived = System.currentTimeMillis();
        sendMsg(msg);
    }
    
    public String getConfParty() {
        return(m_confParty);
    }
     
    
    public Date getUntilTime() {
        return m_untilTime;
    }
    
    public String getFindTerm() {
        return m_findTerm;
    }
    
    private void sendErrMsg(String msg) {
        sendMsg(msg);
        clearContext();        
    }
    
    private String localize(String prompt) {
        return m_localizer.localize(prompt);
    }
    
    private void sendMsg(String msg) {
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
            LOG.error("IMContext.sendMsg XMPP Exception: " + m_chat.getParticipant());
        }         
    }
    
    private void parseFindCmd(StringTokenizer st) {
        if(!st.hasMoreTokens()) {
            sendMsg(localize("find_reply"));
        } else {    
            m_findTerm = st.nextToken();
            m_complete = true;  
        }    
    }
    
    private boolean parseNameOrNumber(StringTokenizer st, boolean ParsingTo) {
        String word = st.nextToken();
        boolean success = false;
        
        if(!ParsingTo) {      
            if("from".startsWith(word)) {               
                if(!st.hasMoreTokens()) {
                    sendMsg(localize("where_reply"));
                    return success;
                } else {
                    word = st.nextToken();  
                }
            }
        }
        
        String  phoneNumber = null;
        Place   place = Place.UNKNOWN;;  
        
        // does token correspond to periods, digits or dashes, if so remove dashes and treat as number
        // if name could be cell, home keywords
        
        if("cell".startsWith(word)) {
            success = true;
            place = Place.CELL;
            phoneNumber = "";
        }
        
        if("home".startsWith(word)) {
            success = true;
            place = Place.HOME;
            phoneNumber = "";
        }
        
        if("work".startsWith(word)) {
            success = true;
            place = Place.WORK;
            phoneNumber = "";
        }

        if(place == Place.UNKNOWN) {
            // assume number is digits, dashes, periods, number sign or asterisk. 
            if (word.matches("[0-9[-.*#]]+") || ParsingTo) {
                success = true;
                phoneNumber = word;
                
            } else {
                sendErrMsg(localize("invalid_number"));
            }
        }
        
        if(success) {
            if(ParsingTo) {
                m_toPlace = place;
                m_toPhoneNumber = phoneNumber;
            } else {
                m_fromPlace = place;
                m_fromPhoneNumber = phoneNumber;
            }
        }
        
        return success;
    }
    
    private boolean parsePlace(StringTokenizer st) {
        
        boolean success = false;
        if(!st.hasMoreTokens()) {
            sendMsg(localize("where_reply"));
        } else {                         
            success = parseNameOrNumber(st, false);    
        }   
        return success;
    }
    
    private void parseCallCmd(StringTokenizer st) {
        if(!st.hasMoreTokens()) {
            sendMsg(localize("call_reply"));
        } else {
            if(parseNameOrNumber(st, true) ) {
                m_command = Command.CALL_FROM;
                if(!st.hasMoreTokens()) {
                    sendMsg(localize("where_reply"));
                } else {    
                    m_complete = parsePlace(st);
                } 
            }    
        }
    }
      
    private boolean parseConfParty(StringTokenizer st) {
        boolean success = true;
        
        if(!st.hasMoreTokens()) {
            sendMsg(localize("conf_party_reply"));
            return false;
        } else {       
            m_confParty = st.nextToken();
            if(m_confParty.equals(localize("all"))) {
              m_confParty = null;
              
            } else {            
                try {
                    Integer.parseInt(m_confParty);                
                } catch (NumberFormatException nfe) {
                    sendErrMsg(localize("invalid_participant"));
                    success = false;
                }       
            }
            return success;
        }
    }
    
    private void parseHistoryCmd(StringTokenizer st) {
        // no matter what, the command is complete
        m_complete = true;   
        m_historyNum = 1; // default is the current day's missed calls
        
        if(st.hasMoreTokens()) {
          // see if next token is a positive integer         
          try {
              int i = Integer.parseInt(st.nextToken().trim());
              if(i > 0) {
                  m_historyNum = i;
              }
           } catch (NumberFormatException nfe) {
              // not an integer so just ignore  
           }              
        }                
    }
    
    public int getHistoryNum() {
        return m_historyNum;
    }
        
    private void sendHelp(StringTokenizer st) {
        
        if(st.hasMoreTokens()) {            
            Command cmd = cmdStrtoCommand(st.nextToken().trim());
              
            switch (cmd) {
            case CALL:    
                sendMsg(localize("call_help_1"));
                sendMsg(localize("call_help_2"));
                sendMsg(localize("call_help_3"));
                sendMsg(localize("call_help_4"));
                sendMsg(localize("call_help_5"));
                break;             
            
            case CONFERENCE:
                sendMsg(localize("conf_help_1"));
                sendMsg(localize("conf_help_2"));
                sendMsg(localize("conf_help_3"));
                sendMsg(localize("conf_help_4"));
                break;
            
            case FIND:    
                sendMsg(localize("find_help_1"));
                sendMsg(localize("find_help_2"));
                sendMsg(localize("find_help_3"));
                break;
            
            case HISTORY:    
                sendMsg(localize("history_help"));
                break;
                        
            case MUTE_CONF_PARTY:    
                sendMsg(localize("mute_help_1"));
                sendMsg(localize("mute_help_2"));
                sendMsg(localize("mute_help_3"));
                sendMsg(localize("mute_help_4"));
                break;   
            
            case UNMUTE_CONF_PARTY: 
                sendMsg(localize("unmute_help_1"));
                sendMsg(localize("unmute_help_2"));
                sendMsg(localize("unmute_help_3"));
                sendMsg(localize("unmute_help_4"));
                break;   
            
            case DISC_CONF_PARTY:
                sendMsg(localize("disc_conf_help_1"));
                sendMsg(localize("disc_conf_help_2"));
                sendMsg(localize("disc_conf_help_3"));
                sendMsg(localize("disc_conf_help_4"));
                break;   
     
            case LOCK_CONF:         
                sendMsg(localize("lock_help"));
                break;   

            case UNLOCK_CONF:
                sendMsg(localize("unlock_help")); 
                break;
            
            case WHO:
                sendMsg(localize("who_help"));
                break;   

            case PICKUP:            
                sendMsg(localize("pickup_help"));
                break;
            
            case LISTENIN:
                sendMsg(localize("listen_help"));
                break;
            
            case NONE:
                sendMsg(localize("do_not_understand"));
                break;
            }    
            return;
            
        }     
        
        sendMsg("Commands: call, find, missed, listen, pickup, conference, " +
                "who, mute, unmute, disconnect, lock, unlock or any short forms");
        
        sendMsg("Type help followed by a command for details (e.g. 'help call')");                  
    }
    
    private Command cmdStrtoCommand(String cmd) {
        
        int matches = 0;
        Command result = Command.NONE;
        
        if(localize("call").startsWith(cmd)) {
            result = Command.CALL;        
            matches++;
        }
        
        if(localize("conference").startsWith(cmd)) {
            result = Command.CONFERENCE;      
            matches++;
        }

        if(localize("missed").startsWith(cmd)) {
            result = Command.HISTORY;
            matches++;
        }
        
        if(localize("help").startsWith(cmd)) {
            result = Command.HELP;
        }
        
        if(localize("find").startsWith(cmd)) {
            matches++;
            result = Command.FIND;
        }
                           
        if(localize("mute").startsWith(cmd)) {
            matches++;
            result = Command.MUTE_CONF_PARTY;   
        }
        
        if(localize("unmute").startsWith(cmd)) {
            matches++;
            result = Command.UNMUTE_CONF_PARTY;   
        }
        
        if(localize("disconnect").startsWith(cmd)) {
            matches++;
            result = Command.DISC_CONF_PARTY;   
        }          
        
        if(localize("lock").startsWith(cmd)) {
            result = Command.LOCK_CONF;
            matches++;
        }
        
        if(localize("unlock").startsWith(cmd)) {
            result = Command.UNLOCK_CONF;
            matches++;
        }
        
        if(localize("who").startsWith(cmd)) {
            result = Command.WHO;
            matches++;
        }
        
        if(localize("pickup").startsWith(cmd)) {
            result = Command.PICKUP;
            matches++;
        }
        
        if(localize("listen").startsWith(cmd)) {
            result = Command.LISTENIN;
            matches++;
        }
        
        if(matches != 1) {
            result = Command.NONE;
        }
        
        return result;
    }        

    private void parseIM(String rcvIM) {
        
        int matches = 0;
        
        rcvIM.trim();
        rcvIM = rcvIM.toLowerCase();
        
        StringTokenizer st = new StringTokenizer(rcvIM);
        
        int numTokens = st.countTokens();
        if(numTokens == 0) return;
                
        if(m_command == Command.NONE) {
            String firstWord = st.nextToken(); 

            if(localize("call").startsWith(firstWord)) {
                m_command = Command.CALL;        
                matches++;
            }
            
            if(localize("conference").startsWith(firstWord)) {
                m_toPlace = Place.CONFERENCE;
                m_command = Command.CALL_FROM;      
                matches++;
            }

            if(localize("missed").startsWith(firstWord)) {
                m_command = Command.HISTORY;
                matches++;
            }
            
            if(localize("help").startsWith(firstWord)) {
                // we deal with help ourselves 
                sendHelp(st);
                return;
            }
            
            if(localize("find").startsWith(firstWord)) {
                matches++;
                m_command = Command.FIND;
            }
            
            /* for now don't support
            if("unblock".startsWith(firstWord)) {
                m_command = Command.UNBLOCK;
                m_complete = true;
            }

            if("block".startsWith(firstWord)) {
                m_command = Command.BLOCK;               
            }
            
            if("at".startsWith(firstWord)) {
                m_command = Command.AT;   
            }
            
                        
            if("status".startsWith(firstWord)) {
                m_command = Command.STATUS;
                m_complete = true;
            }
            */  
                        
            if(localize("mute").startsWith(firstWord)) {
                matches++;
                m_command = Command.MUTE_CONF_PARTY;   
            }
            
            if(localize("unmute").startsWith(firstWord)) {
                matches++;
                m_command = Command.UNMUTE_CONF_PARTY;   
            }
            
            if(localize("disconnect").startsWith(firstWord)) {
                matches++;
                m_command = Command.DISC_CONF_PARTY;   
            }          
            
            if(localize("lock").startsWith(firstWord)) {
                m_complete = true;
                m_command = Command.LOCK_CONF;
                matches++;
            }
            
            if(localize("unlock").startsWith(firstWord)) {
                m_complete = true;
                m_command = Command.UNLOCK_CONF;
                matches++;
            }
            
            if(localize("who").startsWith(firstWord)) {
                m_complete = true;
                m_command = Command.WHO;
                matches++;
            }
            
            if(localize("pickup").startsWith(firstWord)) {
                m_command = Command.PICKUP;
                m_complete = true;
                matches++;
            }
            
            if(localize("listen").startsWith(firstWord)) {
                m_command = Command.LISTENIN;
                matches++;
                m_complete = true;
            }
            
            if(matches != 1) {
                m_command = Command.NONE;
                m_complete  = false;
                sendMsg(localize("do_not_understand"));
            }
        }        
        
        if(!m_complete && m_command != Command.NONE) {
            // in the middle of a command
            switch (m_command) {
                case CALL:
                   parseCallCmd(st);
                   break;
                                      
                case CALL_FROM:
                    m_complete = parsePlace(st);
                    break;
                                       
                case FIND:
                   parseFindCmd(st);
                   break;

                /*   
                case AT:
                    parseAtCmd(st);
                    break;                     
                    
                case AT_UNTIL:
                    m_complete = parseTime(st);
                    break;
   
                case BLOCK:
                    parseBlockCmd(st);
                    break;
                    
                case BLOCK_UNTIL:
                    m_complete = parseTime(st);
                    break;
                */
                case HISTORY:
                    parseHistoryCmd(st); 
                    break;                  
                    
                case REJECTCALL:
                    // nothing to parse
                    m_complete = true;
                    break;   
                    
                case MUTE_CONF_PARTY:
                    m_complete = parseConfParty(st);
                    break;
                    
                case DISC_CONF_PARTY:
                    m_complete = parseConfParty(st);
                    break;
                    
                case UNMUTE_CONF_PARTY:
                    m_complete = parseConfParty(st);
                    break;                    
            }
        } 
    }
    
    public boolean receivedIM(String rcvIM) {              
        long currtime = System.currentTimeMillis();
       
        if(m_command != Command.NONE) {
        
            // check to see if the last IM sent is "too old"
            if(currtime - m_timeReceived > s_TIMEOUT) {
                m_command = Command.NONE;                
            }            
        }
        
        // remember the time of the last IM received
        m_timeReceived = currtime;        
        parseIM(rcvIM);
        
        return m_complete;
    }

    public void setResource(String resource) {
        m_resource = resource;
    }
    
    /*
    private Date midnight(Date date) {
        Calendar cal = new GregorianCalendar();
        cal.setTime(date);
        cal.set(Calendar.HOUR_OF_DAY, 0);
        cal.set(Calendar.MINUTE, 0);
        cal.set(Calendar.SECOND, 0);
        cal.set(Calendar.MILLISECOND, 0);
        cal.add(Calendar.DAY_OF_MONTH, 1);
               
        return cal.getTime();
    }
    
    private boolean parseTime(StringTokenizer st) {        
        int minute = 0;
        
        String word = st.nextToken();
        // meridian may be separated by a space
        if(st.hasMoreTokens()) {
            word += st.nextToken();
        }
        
        Date now = new Date();
        
        // expect the token to be of the form tomorrow, or 3am or 3:04pm or 4.40am or 16:10 (24 hour clock)
        // time had better be in the future 
        if("tomorrow".startsWith(word)) {
            m_untilTime = midnight(now);          // midnight
            return true;
        } 
                
        boolean success = false;
        String errMsg = "That time is not valid.";
        
        VALIDATION: {
            
            Pattern pattern = Pattern.compile("([0-9]{1,2})(:[0-9]{2})?(.*)");
            Matcher matcher = pattern.matcher(word);
            if(matcher.matches()) {

 
                int hour = Integer.parseInt(matcher.group(1));
                if(matcher.group(2) != null) {
                    minute = Integer.parseInt(matcher.group(2).substring(1)); // skip the ":" character
                } 
            
                String meridian = matcher.group(3);
           
                if(meridian != null) {
                    if(meridian.equalsIgnoreCase("pm")) {
                        if(hour < 12) {
                            hour += 12;
                        } else {
                            break VALIDATION;
                        }  
                    } else if(!meridian.equalsIgnoreCase("am")) {
                        break VALIDATION;
                    }                                    
                } 
           
                if(minute > 59) {
                    break VALIDATION;
                }
           
                // now create a Date out of the input
                Calendar cal = new GregorianCalendar();
                cal.setTime(now);
                cal.set(Calendar.HOUR_OF_DAY, hour);
                cal.set(Calendar.MINUTE, minute);
                cal.set(Calendar.SECOND, 0);
                cal.set(Calendar.MILLISECOND, 0);
           
                Date untilTime = cal.getTime();
                if(now.after(untilTime)) {
                    errMsg = "That time is in the past.";  
                    break VALIDATION;
                } else {
                    m_untilTime = untilTime;
                }
            } else {
                break VALIDATION;
            }        
            success = true;
        }
        
        if(!success) {
            sendErrMsg(errMsg);
        }
            
        return success;
    }
  
    private boolean parseUntil(StringTokenizer st) {
        String word = st.nextToken();        
        boolean success = false;
        
        if("until".startsWith(word)) {
            if(m_command == Command.AT) {
                m_command = Command.AT_UNTIL;
            }
            
            if(m_command == Command.BLOCK) {
                m_command = Command.BLOCK_UNTIL;
            }
 
            if(st.hasMoreTokens()) {
                success = parseTime(st);
            } else {
                sendMsg("Reply with:  <time>. For example: 3pm");
            }
        } else {
            sendErrMsg("I do not understand");
        }
        return success;
    }

    private void parseBlockCmd(StringTokenizer st) {
        if(!st.hasMoreTokens()) {
            sendMsg("Reply with:  *until <time>*. For example: *until 3pm*");
        }   
        
        m_complete = parseUntil(st);  
    }

    private void parseAtCmd(StringTokenizer st) {
       
        if(!st.hasMoreTokens()) {
            sendMsg("Reply with: cell, home, work or number. For example: 234-5678");
        } else {                   
            boolean success = parseNameOrNumber(st, false);    
            
            if(success && st.hasMoreTokens()) {
                success = parseUntil(st);                    
            }
            
            // if we could parse a name or number and optionally could parse an until time then
            // we are complete
            m_complete = success;
        }    
    }
    */

}
