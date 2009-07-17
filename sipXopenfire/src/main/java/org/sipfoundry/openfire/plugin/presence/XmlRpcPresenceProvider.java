package org.sipfoundry.openfire.plugin.presence;

import java.util.HashMap;
import java.util.Map;

import org.apache.log4j.Logger;
import org.sipfoundry.sipcallwatcher.ResourceState;
import org.xmpp.component.ComponentManager;
import org.xmpp.component.ComponentManagerFactory;
import org.xmpp.packet.Presence;
import  org.xmpp.component.Log;

public class XmlRpcPresenceProvider extends XmlRpcProvider {
 
    public static final String SERVICE = "status";
    
    public static final String SERVER = "presenceServer";

   
    
    private Logger log = Logger.getLogger(XmlRpcPresenceProvider.class);
    
    

  

    public XmlRpcPresenceProvider() {
  
    }

    public Map getPresenceState( String id) {
        try {
            String jid = appendDomain(id);
            log.info("GetPresenceState " + jid);
            Map retval = createSuccessMap();
            Presence presence = plugin.getPresence(jid, jid);
            if ( presence == null ) {
                retval.put(XMPP_PRESENCE, PresenceState.OFFLINE);
            } else if ( presence.getShow() == null ) {
                retval.put(XMPP_PRESENCE, PresenceState.ONLINE);
            } else {
                retval.put(XMPP_PRESENCE, presence.getShow().toString());
            }
            return retval;
        } catch (Exception e) {
            log.error("User Not Found",e);
            return createErrorMap(ErrorCode.USER_NOT_FOUND, e.getMessage());
        }
    }
    
    public Map setPresenceState (String id , String show) {
        try {
            String jid = appendDomain(id);
            log.info("setPresenceState" + jid + " Show " + show);
            Map retval = createSuccessMap();
            plugin.setPresenceState(jid,  show);   
            return retval;
        } catch ( Exception ex) {
            log.error("User Not Found",ex);
            return createErrorMap(ErrorCode.USER_NOT_FOUND, ex.getMessage());
        }
    }
    
    public Map setPresenceStatus(String id, String status) {
        try {
            String jid = appendDomain(id);
            log.info("setPresenceStatus " + jid + " status = " + status);
            Map retval = createSuccessMap();
            plugin.setPresenceStatus(jid,  status);   
            return retval;
        } catch ( Exception ex) {
            log.error("User Not Found",ex);
            return createErrorMap(ErrorCode.USER_NOT_FOUND, ex.getMessage());
        }
    }
    
    public Map getPresenceStatus(String id) {
        try {
            String jid = appendDomain(id);
            log.info("getPresenceStatus " + jid) ;
            Map retval = createSuccessMap();
            String presenceStatus = plugin.getPresenceStatus(jid);
            if ( presenceStatus != null ) {
                retval.put(CUSTOM_PRESENCE_MESSAGE, presenceStatus);
            }
            return retval;
        } catch (Exception ex) {
            log.error("UserNotFound ", ex);
            return createErrorMap(ErrorCode.USER_NOT_FOUND,ex.getMessage());
        }
    }
    /**
     * @param sipId  SIP identity for which presence info is requested
     *
     *@return a Map containing the following information:
     *
     * RESPONSE
     * If response reports a failure, a map with the following three elements is returned:
     * "status-code"  // value == "ERROR"
     * "faultCode"    // code 1 means SIP identity not found
     * "faultString"  // plaintext explaination of failure

     * If response reports a success, a map with the following seven elements is returned:
     *    "status-code"  // value == "OK
     *    "sip-resource-id" // queried SIP identity
     *    "jabber-id"       // associated jabber id
     *    "telephony-presence" // string representing telephony presence.  Can be "idle", "busy" or "undetermied"
     *    "xmpp-presence" // string representing XMPP presence.  Can be "available", "away", "xa", "dnd" or "offline"
     *    "unified-presence" // string representing unified XMPP presence which is a merge of the telephony and XMPP presences
     *     "custom-presence-message" // string representing user-supplied cusomt message linked to presence state.  Can be an empty string
     *
     */
    public Map getUnifiedPresenceInfo( String sipId ) {
        try {
           log.info("getUnifiedPresenceInfo " + sipId);
           String xmppUser = plugin.getXmppId(sipId);
           if ( xmppUser == null ) {
              Map errorMap = createErrorMap(ErrorCode.SIP_ID_NOT_FOUND, "SIP ID Not found in database.");
              return errorMap;
           }
           log.debug("xmppUser = " + xmppUser);
           Map retval = createSuccessMap();
           String jid = xmppUser;
           Presence presence = plugin.getPresence(jid, jid);
           String xmppPresence;
           if ( presence == null ) {
               xmppPresence =  PresenceState.OFFLINE;
           } else if ( presence.getShow() == null ) {
               xmppPresence =  PresenceState.ONLINE;
           } else {
               xmppPresence =  presence.getShow().toString();
           }
           retval.put(XMPP_PRESENCE, xmppPresence);
           retval.put(JABBER_ID, jid);
           retval.put(SIP_ID, sipId);
           String sipPresence =  plugin.getSipPresence(sipId);
           retval.put(SIP_PRESENCE, sipPresence);
           String unifiedPresence = UnifiedPresence.AVAILABLE_FOR_BOTH;
           if ( sipPresence.equals(ResourceState.BUSY.toString())  && 
                xmppPresence.equals(PresenceState.OFFLINE)) {
               unifiedPresence = UnifiedPresence.NOT_AVAILABLE.toString();      
           } else if ( sipPresence.equals(ResourceState.IDLE.toString())  && 
                   xmppPresence.equals(PresenceState.AWAY.toString())){
               unifiedPresence = UnifiedPresence.AVAILABLE_FOR_PHONE;         
           } else if ( sipPresence.equals(ResourceState.BUSY) && 
                   xmppPresence.equals(PresenceState.CHAT)) {
               unifiedPresence = UnifiedPresence.AVAILABLE_FOR_CHAT;
           } else if ( sipPresence.equals(ResourceState.IDLE) && 
                   xmppPresence.equals(PresenceState.ONLINE)) {
               unifiedPresence = UnifiedPresence.AVAILABLE_FOR_BOTH;
           } 
           retval.put(UNIFIED_PRESENCE, unifiedPresence);
           String presenceStatus = plugin.getPresenceStatus(jid);
           retval.put(CUSTOM_PRESENCE_MESSAGE, presenceStatus);
           return retval;
        } catch (Exception ex) {
            log.error("Processing error",ex);
            return createErrorMap(ErrorCode.PROCESSING_ERROR,ex.getMessage());
        }
        
    }
    
    
    
    
    

}
