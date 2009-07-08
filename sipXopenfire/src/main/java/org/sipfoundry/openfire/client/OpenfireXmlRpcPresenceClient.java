package org.sipfoundry.openfire.client;

import java.net.URL;
import java.util.Map;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLSession;

import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.Logger;
import org.apache.log4j.SimpleLayout;
import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.client.XmlRpcClient;
import org.apache.xmlrpc.client.XmlRpcClientConfigImpl;
import org.sipfoundry.openfire.plugin.presence.XmlRpcPresenceProvider;
import org.sipfoundry.openfire.plugin.presence.XmlRpcUserAccountProvider;

public class OpenfireXmlRpcPresenceClient  extends OpenfireXmlRpcClient {
    private boolean isSecure = false;
    private XmlRpcClient client;
    private String serverAddress;
    
    private static Logger logger = Logger.getLogger(OpenfireXmlRpcPresenceClient.class);
    
    static {
        logger.addAppender(new ConsoleAppender(new SimpleLayout()));
    }
    
    

    public OpenfireXmlRpcPresenceClient(String serverAddress, int port) throws Exception {
        XmlRpcClientConfigImpl config = new XmlRpcClientConfigImpl();
        config.setEnabledForExceptions(true);
        config.setEnabledForExtensions(true);
        this.serverAddress = serverAddress;
        String url = (isSecure ? "https" : "http") + "://" + serverAddress + ":" + port+"/plugins/sipx-openfire/" + XmlRpcPresenceProvider.SERVICE;
        config.setServerURL(new URL(url));
        this.client = new XmlRpcClient();
        this.client.setConfig(config);
    }
    
    public String getXmppPresenceState(String userName) throws OpenfireClientException {
        
        Object[] args = new Object[1];
        args[0] = userName + "@" + serverAddress;
      
        Map retval;
        try {
            retval = (Map) client.execute(XmlRpcPresenceProvider.SERVER + ".getPresenceState", args);
        } catch (XmlRpcException e) {
            throw new OpenfireClientException(e);
        }
        if (retval.get(XmlRpcPresenceProvider.STATUS_CODE).equals(XmlRpcPresenceProvider.ERROR)) {
            throw new OpenfireClientException("Error in processing request "
                    + retval.get(XmlRpcPresenceProvider.ERROR_INFO));
        }
        return (String) retval.get(XmlRpcPresenceProvider.XMPP_PRESENCE);   
        
    }
    
    public void setXmppPresenceState(String userName, String state) throws OpenfireClientException {
        Object[] args = new Object[2];
        args[0] = userName + "@" + serverAddress;
        args[1] = state;
        Map retval ;
        try {
            retval = (Map)client.execute(XmlRpcPresenceProvider.SERVER + ".setPresenceState", args);
        }catch (XmlRpcException e) {
            throw new OpenfireClientException(e);
        }
        if (retval.get(XmlRpcPresenceProvider.STATUS_CODE).equals(XmlRpcPresenceProvider.ERROR)) {
            throw new OpenfireClientException("Error in processing request "
                    + retval.get(XmlRpcPresenceProvider.ERROR_INFO));
        }
    }
    
    
    public void setXmppCustomPresenceMessage(String userName, String statusMessage) throws OpenfireClientException {
        Object[] args = new Object[2];
        args[0] = userName + "@" + serverAddress;
        args[1] = statusMessage;
        Map retval ;
        try {
            retval = (Map)client.execute(XmlRpcPresenceProvider.SERVER + ".setPresenceStatus", args);
        }catch (XmlRpcException e) {
            throw new OpenfireClientException(e);
        }
        if (retval.get(XmlRpcPresenceProvider.STATUS_CODE).equals(XmlRpcPresenceProvider.ERROR)) {
            throw new OpenfireClientException("Error in processing request "
                    + retval.get(XmlRpcPresenceProvider.ERROR_INFO));
        }
    }
    
    
    
    public String getXmppCustomPresenceMessage(String userName) throws OpenfireClientException {
        Object[] args = new Object[1];
        args[0] = userName ;
        Map retval;
        try {
            retval = (Map) client.execute(XmlRpcPresenceProvider.SERVER + ".getPresenceStatus", args);
        } catch (XmlRpcException ex) {
            throw new OpenfireClientException(ex);
        }
        if (retval.get(XmlRpcPresenceProvider.STATUS_CODE).equals(XmlRpcPresenceProvider.ERROR)) {
            throw new OpenfireClientException("Error in processing request "
                    + retval.get(XmlRpcPresenceProvider.ERROR_INFO));
        }
        return (String) retval.get(XmlRpcPresenceProvider.CUSTOM_PRESENCE_MESSAGE);
    }
    
    /**
     * Method name:  "sipXopenfire.getUnifiedPresenceInfo"
     * 
     * @param  SIP identity for which presence info is requested
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
    public Map getUnifiedPresenceInfo(String sipIdentity) throws OpenfireClientException {
        Object[] args = new Object[1];
        args[0] = sipIdentity;
        Map retval;
        try {
            retval = (Map) client.execute(XmlRpcPresenceProvider.SERVER + ".getUnifiedPresenceInfo", args);
        } catch ( Exception ex) {
            throw new OpenfireClientException(ex);
        }
        return retval;
        
    }
    

}
