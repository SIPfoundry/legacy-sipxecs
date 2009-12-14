package org.sipfoundry.openfire.plugin.presence;

import java.util.Map;
import java.util.Random;
import org.apache.log4j.Logger;
import org.xmpp.packet.JID;

public class XmlRpcPresenceProvider extends XmlRpcProvider {
 
    private static String myHandle = "of:" + Math.abs(new Random().nextLong());

    public static final String SERVICE_PATH = "/plugins/sipx-openfire-presence/status";
    public static final String SERVICE_NAME = "status";
    public static final String SERVER = "presenceServer";

    private Logger log = Logger.getLogger(XmlRpcPresenceProvider.class); 

    public XmlRpcPresenceProvider() {
  
    }

    public Map getPresenceState( String xmppUsername ) {
        try {
            String jid = appendDomain(xmppUsername);
            log.info("GetPresenceState " + jid);
            Map retval = createSuccessMap();
            UnifiedPresence unifiedPresence = 
                  PresenceUnifier.getInstance().getUnifiedPresence( xmppUsername );
            retval.put(XMPP_PRESENCE, unifiedPresence.getXmppPresence().toString());
            return retval;
        } catch (IllegalArgumentException e) {
            log.error("Invalid enum value ",e);
            return createErrorMap(ErrorCode.INVALID_XMPP_PRESENCE_VALUE, e.getMessage());
        } catch (Exception e) {
            log.error("User Not Found",e);
            return createErrorMap(ErrorCode.USER_NOT_FOUND, e.getMessage());
        }
    }
    
    public Map setPresenceState (String xmppUsername , String xmppPresenceAsString ) {
        try {
            String jid = appendDomain(xmppUsername);
            log.info("setPresenceState" + jid + " XMPP presence " + xmppPresenceAsString);
            Map retval = createSuccessMap();
            UnifiedPresence.XmppPresence xmppPresence = UnifiedPresence.XmppPresence.valueOf( xmppPresenceAsString ); 
            getPlugin().setPresenceState(jid, xmppPresence);   
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
            getPlugin().setPresenceStatus(jid,  status);   
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
            String presenceStatus = getPlugin().getPresenceStatus(jid);
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
     *    "sip-resource-id" // queried SIP identity (user part only)
     *    "jabber-id"       // associated jabber id
     *    "telephony-presence" // string representing telephony presence.  Can be "IDLE", "BUSY" or "UNDERMINED"
     *    "xmpp-presence" // string representing XMPP presence.  Can be "AVAILABLE", "OFFLINE", "EXTENDED_AWAY", "AWAY", "BUSY" and "CHAT"
     *    "unified-presence" // string representing unified XMPP presence which is a merge of the telephony and XMPP presences. Can be:
     *                       // "not-available", "available-for-phone", "available-for-chat" and "available-for-both"
     *    "custom-presence-message" // string representing user-supplied cusomt message linked to presence state.  Can be an empty string
     *
     */
    public Map getUnifiedPresenceInfo( String sipId ) {
        try {
           log.info("getUnifiedPresenceInfo " + sipId);
           String xmppUser = getPlugin().getXmppId(sipId);
           if ( xmppUser == null ) {
              Map errorMap = createErrorMap(ErrorCode.SIP_ID_NOT_FOUND, "SIP ID Not found in database.");
              return errorMap;
           }
           log.debug("xmppUser = " + xmppUser);
           Map retval = createSuccessMap();
           JID jid = new JID(xmppUser);
           UnifiedPresence unifiedPresence 
                   = PresenceUnifier.getInstance().getUnifiedPresence( jid.getNode());
           String xmppPresence = unifiedPresence.getXmppPresence().toString();

           retval.put(XMPP_PRESENCE, unifiedPresence.getXmppPresence().toString());
           retval.put(JABBER_ID, jid.toString());
           retval.put(SIP_ID, sipId);
           retval.put(SIP_PRESENCE, unifiedPresence.getSipState().toString());
           retval.put(UNIFIED_PRESENCE, unifiedPresence.getUnifiedPresence());
           retval.put(CUSTOM_PRESENCE_MESSAGE, unifiedPresence.getXmppStatusMessageWithSipState());
           return retval;
        } catch (Exception ex) {
            log.error("Processing error",ex);
            return createErrorMap(ErrorCode.PROCESSING_ERROR,ex.getMessage());
        }
        
    }
    
    public Map registerPresenceMonitor(String protocol, String serverUrl ) 
    {
        log.info("registerPresenceMonitor " + protocol + " " + serverUrl );            
        PresenceUnifier.getInstance().addUnifiedPresenceChangeListener( protocol, serverUrl );
        Map retval = createSuccessMap();
        retval.put(INSTANCE_HANDLE, myHandle);
        return retval;
    }
     
    public Map ping( String originatorName ) 
    {
        log.info("ping received from " + originatorName );            
        Map retval = createSuccessMap();
        retval.put(INSTANCE_HANDLE, myHandle);
        return retval;
    }
    

}
