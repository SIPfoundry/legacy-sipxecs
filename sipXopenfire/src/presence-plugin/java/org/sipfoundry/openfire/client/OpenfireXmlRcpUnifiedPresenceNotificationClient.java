/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.client;

import java.net.URL;
import java.util.Map;
import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.Logger;
import org.apache.log4j.SimpleLayout;
import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.client.XmlRpcClient;
import org.apache.xmlrpc.client.XmlRpcClientConfigImpl;
import org.sipfoundry.openfire.plugin.presence.PresenceUnifier;
import org.sipfoundry.openfire.plugin.presence.SipXOpenfirePlugin;
import org.sipfoundry.openfire.plugin.presence.UnifiedPresence;
import org.sipfoundry.openfire.plugin.presence.UnifiedPresence;
import org.sipfoundry.openfire.plugin.presence.UnifiedPresenceChangeListener;
import org.sipfoundry.openfire.plugin.presence.XmlRpcPresenceProvider;

public class OpenfireXmlRcpUnifiedPresenceNotificationClient extends OpenfireXmlRpcClient 
                                                             implements UnifiedPresenceChangeListener
                                                              
{


	private boolean isSecure = false;
    private XmlRpcClient client;
    private String serverUrl;
    private SipXOpenfirePlugin plugin;
    private boolean dummy;
    private static Logger logger = Logger.getLogger(OpenfireXmlRcpUnifiedPresenceNotificationClient.class);

    public OpenfireXmlRcpUnifiedPresenceNotificationClient(String serverUrl,
                                                           SipXOpenfirePlugin plugin ) throws Exception 
    {
        this.plugin = plugin;
        XmlRpcClientConfigImpl config = new XmlRpcClientConfigImpl();
        config.setEnabledForExceptions(true);
        config.setEnabledForExtensions(true);
        config.setConnectionTimeout(120000);
        config.setReplyTimeout(120000);
        this.serverUrl = serverUrl;
        config.setServerURL(new URL(serverUrl));
        this.client = new XmlRpcClient();
        this.client.setConfig(config);
	}

    public void unifiedPresenceChanged( String xmppUsername, UnifiedPresence newUnifiedPresence )
    {
        String retval = null;
        try {
            Object[] args = new Object[6];
            String xmppId = newUnifiedPresence.getXmppUsername() + "@" + plugin.getXmppDomain();
            args[0] = newUnifiedPresence.getXmppPresence().toString();
            args[1] = xmppId;
            args[2] = plugin.getSipId( xmppId );
            args[3] = newUnifiedPresence.getSipState().toString();
            args[4] = newUnifiedPresence.getUnifiedPresence();
            args[5] = newUnifiedPresence.getXmppStatusMessage();
            retval = (String)client.execute("UnifiedPresenceChangeListener.unifiedPresenceChanged", args);
        }catch (XmlRpcException e) {
            logger.info( "unifiedPresenceChanged caught XmlRpcException for server " + serverUrl + " : ", e );
            logger.info( "removing change listener as a result" );
            PresenceUnifier.getInstance().removeUnifiedPresenceChangeListener("xmlrpc", serverUrl );
        }catch (Exception e ){
            logger.info( "unifiedPresenceChanged caught exception for server " + serverUrl + " : ", e );
        }
        if (retval != null && !retval.equals("ok"))
        {
            logger.info( "unifiedPresenceChanged returned an error while processing request " + retval );
        }
    }
    
    @Override
	public boolean equals(Object obj) {
    	return ( obj instanceof Integer ) && 
               ( serverUrl.equals(((OpenfireXmlRcpUnifiedPresenceNotificationClient) obj).serverUrl ));
	}
	@Override
	public int hashCode() {
		return serverUrl.hashCode();
	}
	
}
