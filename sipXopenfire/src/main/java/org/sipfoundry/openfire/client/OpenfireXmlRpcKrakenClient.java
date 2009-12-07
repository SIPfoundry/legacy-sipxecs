package org.sipfoundry.openfire.client;

import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Map;

import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.Logger;
import org.apache.log4j.SimpleLayout;
import org.apache.xmlrpc.XmlRpcException;
import org.sipfoundry.openfire.config.XmppTransportRegistration;
import org.xmpp.packet.JID;

public class OpenfireXmlRpcKrakenClient  extends OpenfireXmlRpcClient {
    
    private static Logger logger = Logger.getLogger(OpenfireXmlRpcKrakenClient.class);
    
    public static final String KRAKEN_PLUGIN_PASSWORD = "sipXecs";
    public static final String KRAKEN_ERROR_AUTH_FAILED = "Authorization failed!";
    public static final String KRAKEN_STATUS_SUCCESS = "Success";
    public static final String KRAKEN_USER_NAME = "jid";
    public static final String KRAKEN_TRANSPORT_TYPE = "transportType";
    public static final String KRAKEN_LEGACY_USER_NAME = "userName";
    public static final String KRAKEN_LEGACY_PASSWORD = "password";
    public static final String KRAKEN_LEGACY_NICKNAME = "nickname";
    
    public OpenfireXmlRpcKrakenClient(String serverAddress, int port) throws Exception {
        super("Manager", "/plugins/kraken/xml-rpc" , serverAddress, port);
     }
    
    public Boolean toggleTransport(String transport) throws OpenfireClientException {
        
        Object[] args = new Object[2];
        args[0] = OpenfireXmlRpcKrakenClient.KRAKEN_PLUGIN_PASSWORD;
        args[1] = transport;

    	logger.debug("Kraken XML-RPC toggleTransport for: " + transport);

        Boolean retval;
        try {
        	retval = executeBoolean("toggleTransport", args);

        } catch (XmlRpcException e) {
            throw new OpenfireClientException(e.getMessage());
        }

    	logger.debug("new status for: " + transport + " is " + (retval ? "active" : "inactive"));

        return retval;        
    }

    public List<String> getActiveTransports() throws OpenfireClientException {
        
        Object[] args = new Object[1];
        args[0] = OpenfireXmlRpcKrakenClient.KRAKEN_PLUGIN_PASSWORD;

    	logger.debug("Kraken XML-RPC getActiveTransports");

    	List<String> retval;
        try {
        	retval = executeStringList("getActiveTransports", args);
         	
        	// Check for errors 
        	if (retval.size() == 1) {
            	if (retval.contains(OpenfireXmlRpcKrakenClient.KRAKEN_ERROR_AUTH_FAILED)) {
                    throw new OpenfireClientException("Processing Error: " + 
                    		OpenfireXmlRpcKrakenClient.KRAKEN_ERROR_AUTH_FAILED);
            	}
            }
        } catch (XmlRpcException e) {
            throw new OpenfireClientException(e.getMessage());
        } catch (Throwable t) {
            throw new OpenfireClientException("Processing Error: " + t.getMessage());
        }

    	// dump the list of entries
        for (String transport : retval) {
        	logger.debug("Active transport: " + transport);
        }

        return retval;
    }

    public Collection<XmppTransportRegistration> getRegistrations(String user) throws OpenfireClientException {
        
        Object[] args = new Object[2];
        args[0] = OpenfireXmlRpcKrakenClient.KRAKEN_PLUGIN_PASSWORD;
        args[1] = user;
    	Object[] obj;
    	Collection<XmppTransportRegistration> result = new HashSet<XmppTransportRegistration>();

    	logger.debug("Kraken XML-RPC getRegistrations for user: " + user);

        try {
         	obj = executeObjectArray("getRegistrations", args);

        	// Populate the array of transport registrations
        	for (int i = 0;  i < obj.length;  i++) {
        		Map xmlRpcRegStruct = (Map) obj[i];
        		XmppTransportRegistration registration = new XmppTransportRegistration();

        		// Get the user name into a temporary string because it will need to be
        		// converted from a jabber id format to a user name format.
        		String userName = (String)xmlRpcRegStruct.get(OpenfireXmlRpcKrakenClient.KRAKEN_USER_NAME);
                if (userName == null) {
                    logger.error("getRegistrations: Field not found: " + OpenfireXmlRpcKrakenClient.KRAKEN_USER_NAME);

                    // Skip this entry
                    continue;
                }
                JID jid = new JID(userName);
        		registration.setUser(jid.getNode());

        		registration.setTransportType((String)xmlRpcRegStruct.get(OpenfireXmlRpcKrakenClient.KRAKEN_TRANSPORT_TYPE));
                if (registration.getTransportType() == null) {
                    logger.error("getRegistrations: Field not found: " + OpenfireXmlRpcKrakenClient.KRAKEN_TRANSPORT_TYPE);

                    // Skip this entry
                    continue;
                }

        		registration.setLegacyUsername((String)xmlRpcRegStruct.get(OpenfireXmlRpcKrakenClient.KRAKEN_LEGACY_USER_NAME));
                if (registration.getLegacyUsername() == null) {
                    logger.error("getRegistrations: Field not found: " + OpenfireXmlRpcKrakenClient.KRAKEN_LEGACY_USER_NAME);

                    // Skip this entry
                    continue;
                }

        		registration.setLegacyPassword((String)xmlRpcRegStruct.get(OpenfireXmlRpcKrakenClient.KRAKEN_LEGACY_PASSWORD));
                if (registration.getLegacyPassword() == null) {
                    logger.error("getRegistrations: Field not found: " + OpenfireXmlRpcKrakenClient.KRAKEN_LEGACY_PASSWORD);

                    // Skip this entry
                    continue;
                }

        		registration.setLegacyNickname((String)xmlRpcRegStruct.get(OpenfireXmlRpcKrakenClient.KRAKEN_LEGACY_NICKNAME));
                if (registration.getLegacyNickname() == null) {
                    logger.error("getRegistrations: Field not found: " + OpenfireXmlRpcKrakenClient.KRAKEN_LEGACY_NICKNAME);

                    // Skip this entry
                    continue;
                }

                logger.debug("Found existing registration: " + registration.toString());
                
            	result.add(registration);
            }
        } catch (XmlRpcException e) {
        	logger.error("getRegistrations exception: " + e.getMessage());
            throw new OpenfireClientException(e.getMessage());
        } catch (Throwable t) {
        	logger.error("getRegistrations throwable: " + t.getMessage());
        	throw new OpenfireClientException("Processing Error: " + t.getMessage());
        }
      
        return result;
    }

    public Boolean addRegistration(String user, String transportType, String legacyUsername, String legacyPassword, String legacyNickname) throws OpenfireClientException {
        
        Object[] args = new Object[6];
        args[0] = OpenfireXmlRpcKrakenClient.KRAKEN_PLUGIN_PASSWORD;
        args[1] = user;
        args[2] = transportType;
        args[3] = legacyUsername;
        args[4] = legacyPassword;
        args[5] = legacyNickname;
        
    	logger.info("Kraken XML-RPC addRegistration for user: " + user + " transport: " + transportType);

        String result;
        try {
        	result = (String) executeString("addRegistration", args);

        } catch (XmlRpcException e) {
            throw new OpenfireClientException(e.getMessage());
        }

    	logger.info("addRegistration operation status is: " + result);
    	
        return result.equals(OpenfireXmlRpcKrakenClient.KRAKEN_STATUS_SUCCESS) ? true : false;
    }
    
    public Boolean updateRegistration(String user, String transportType, String legacyUsername, String legacyPassword, String legacyNickname) throws OpenfireClientException {
        
        Object[] args = new Object[6];
        args[0] = OpenfireXmlRpcKrakenClient.KRAKEN_PLUGIN_PASSWORD;
        args[1] = user;
        args[2] = transportType;
        args[3] = legacyUsername;
        args[4] = legacyPassword;
        args[5] = legacyNickname;
        
    	logger.info("Kraken XML-RPC updateRegistration for user: " + user + " transport: " + transportType);

        String result;
        try {
        	result = (String) executeString("updateRegistration", args);

        } catch (XmlRpcException e) {
            throw new OpenfireClientException(e.getMessage());
        }

    	logger.info("updateRegistration operation status is: " + result);
    	
        return result.equals(OpenfireXmlRpcKrakenClient.KRAKEN_STATUS_SUCCESS) ? true : false;
    }

    public Boolean deleteRegistration(String user, String transportType) throws OpenfireClientException {
        
        Object[] args = new Object[3];
        args[0] = OpenfireXmlRpcKrakenClient.KRAKEN_PLUGIN_PASSWORD;
        args[1] = user;
        args[2] = transportType;
        
    	logger.info("Kraken XML-RPC deleteRegistration for user: " + user + " transport: " + transportType);

        String result;
        try {
        	result = (String) executeString("deleteRegistration", args);

        } catch (XmlRpcException e) {
            throw new OpenfireClientException(e.getMessage());
        }

    	logger.info("deleteRegistration operation status is: " + result);
    	
        return result.equals(OpenfireXmlRpcKrakenClient.KRAKEN_STATUS_SUCCESS) ? true : false;
    }
    
}
