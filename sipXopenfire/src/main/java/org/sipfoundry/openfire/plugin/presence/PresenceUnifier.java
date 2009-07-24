package org.sipfoundry.openfire.plugin.presence;

import java.util.Collection;
import java.util.HashMap;
import java.util.concurrent.ConcurrentHashMap;

import org.sipfoundry.openfire.client.OpenfireXmlRcpUnifiedPresenceNotificationClient;
import org.sipfoundry.sipcallwatcher.SipResourceState;
import org.apache.log4j.Logger;
import org.jivesoftware.openfire.session.ClientSession;
import org.jivesoftware.openfire.user.PresenceEventDispatcher;
import org.jivesoftware.openfire.user.PresenceEventListener;
import org.jivesoftware.openfire.user.UserNotFoundException;
import org.xmpp.packet.JID;
import org.xmpp.packet.Presence;


/**
 * Singleton object that is meant to collect all the presence-related
 * information related the openfire users on the system.  More specifically,
 * by virtue of being a PresenceEventListener, it will receive notifications
 * whenever openfire users change their XMPP presence or status message via their 
 * XMPP clients.  
 * 
 * This object also collects the SIP state information related to the 
 * SIP user linked to openfire users.  Any sub=component of this plug-in
 * that wants to change the SIP state of a user must inform this object
 * via its sipStateChanged() method.
 * 
 * Finally, this object aggregates the XMPP presence and SIP state into
 * a 'unified presence' which can be queried via the getUnifiedPresence()
 * method.
 */
public class PresenceUnifier implements PresenceEventListener 
{
    private static PresenceUnifier instance = null;
    private static SipXOpenfirePlugin sipXopenfirePlugin = null;
    private static Logger log = Logger.getLogger(PresenceUnifier.class);
    private ConcurrentHashMap<String, UnifiedPresence> unifiedPresenceMap = new ConcurrentHashMap<String, UnifiedPresence>();
    private HashMap<String, UnifiedPresenceChangeListener> changeListenerMap = new HashMap<String, UnifiedPresenceChangeListener>();
    
    public static void setPlugin( SipXOpenfirePlugin plugin )
    {
    	sipXopenfirePlugin = plugin; 
    }
    
    public static PresenceUnifier getInstance() 
    {
        if( instance == null ) 
        {
           instance = new PresenceUnifier();
        }
        return instance;
    }
    
    public UnifiedPresence getUnifiedPresence( String xmppUsername ) throws UserNotFoundException
    {
        UnifiedPresence unifiedPresence = unifiedPresenceMap.get( xmppUsername );
        if( unifiedPresence == null )
        {
            throw new UserNotFoundException("No presence information for XMPP user");
        }
        return unifiedPresence;
    }
    
    // Notification that SIP state changed (idle, on-call, ...)
    public void sipStateChanged( String xmppUsername, SipResourceState newState )
    {
        UnifiedPresence unifiedPresence;
        unifiedPresence = unifiedPresenceMap.get( xmppUsername );
        if( unifiedPresence == null )
        {
            unifiedPresence = new UnifiedPresence( xmppUsername );
            unifiedPresenceMap.put( xmppUsername,  unifiedPresence );
        }
        unifiedPresence.setSipState( newState );
        notifyListenersOfChange( unifiedPresence );
        log.debug("SipStateChanged for " + xmppUsername + " new state: " + newState + " new unified presence: " + unifiedPresence.getUnifiedPresence() );
    }

    public void xmppStatusMessageChanged( String xmppUsername, String newXmppStatusMessage )
    {
        UnifiedPresence unifiedPresence;
        unifiedPresence = unifiedPresenceMap.get( xmppUsername );
        if( unifiedPresence == null )
        {
            unifiedPresence = new UnifiedPresence( xmppUsername );
            unifiedPresenceMap.put( xmppUsername,  unifiedPresence );
        }
        unifiedPresence.setXmppStatusMessage( newXmppStatusMessage );
        notifyListenersOfChange( unifiedPresence );
        log.debug("XmppStatusMessageChanged for " + xmppUsername + " new XMPP status message: " + newXmppStatusMessage );
    }
    
    // Notification that XMPP presence changed
    public void presenceChanged(ClientSession session, Presence presence)
    {
        try
        {
            UnifiedPresence unifiedPresence = 
                handleXmppPresenceChange( session.getUsername(), presence );

            notifyListenersOfChange( unifiedPresence );
            log.debug("presenceChanged[1] for " + session.getUsername() 
                    + " new presence show: " + presence.getShow()
                    + " new presence type: " + presence.getType()
                    + " new presence status: " + presence.getStatus()
                    + " new unified presence: " + unifiedPresence.getUnifiedPresence() );        
        }
        catch( UserNotFoundException ex )
        {
            log.debug( "unavailableSession caught " + ex.getMessage() );
        }
    }
    
    // Notification that XMPP presence changed
    public void presenceChanged(String xmppUsername, Presence presence)
    {
        UnifiedPresence unifiedPresence = 
            handleXmppPresenceChange( xmppUsername, presence );

        notifyListenersOfChange( unifiedPresence );
        log.debug("presenceChanged[2] for " + xmppUsername 
                + " new presence show: " + presence.getShow()
                + " new presence type: " + presence.getType()
                + " new presence status: " + presence.getStatus()
                + " new unified presence: " + unifiedPresence.getUnifiedPresence() );        
    }
    
    public void availableSession(ClientSession session, Presence presence)
    {
        try
        {
            UnifiedPresence unifiedPresence = 
                handleXmppPresenceChange( session.getUsername(), presence );

            notifyListenersOfChange( unifiedPresence );
            log.debug("availableSession for " + session.getUsername() 
                    + " new presence show: " + presence.getShow()
                    + " new presence type: " + presence.getType()
                    + " new presence status: " + presence.getStatus()
                    + " new unified presence: " + unifiedPresence.getUnifiedPresence() );        
        }
        catch( UserNotFoundException ex )
        {
            log.debug( "unavailableSession caught " + ex.getMessage() );
        }
    }
   
    public void unavailableSession(ClientSession session, Presence presence)
    {
        try
        {
            UnifiedPresence unifiedPresence = 
                handleXmppPresenceChange( session.getUsername(), presence );

            notifyListenersOfChange( unifiedPresence );
            log.debug("unavailableSession for " + session.getUsername() 
                    + " new presence show: " + presence.getShow()
                    + " new presence type: " + presence.getType()
                    + " new presence status: " + presence.getStatus()
                    + " new unified presence: " + unifiedPresence.getUnifiedPresence() );        
        }
        catch( UserNotFoundException ex )
        {
            log.debug( "unavailableSession caught " + ex.getMessage() );
        }
    }
        
    
    public void subscribedToPresence(JID subscriberJID, JID authorizerJID)
    {
        
    }

    public void unsubscribedToPresence(JID subscriberJID, JID authorizerJID)
    {
        
    }

    private UnifiedPresence handleXmppPresenceChange( String xmppUsername, Presence presence )
    {
        UnifiedPresence unifiedPresence;
        unifiedPresence = unifiedPresenceMap.get( xmppUsername );
        if( unifiedPresence == null )
        {
            unifiedPresence = new UnifiedPresence( xmppUsername );
            unifiedPresenceMap.put( xmppUsername,  unifiedPresence );
        }
        unifiedPresence.setXmppPresence( presence );
        return unifiedPresence; 
    }
    
    private PresenceUnifier()
    {
        PresenceEventDispatcher.addListener( this );
    }

    synchronized public void addUnifiedPresenceChangeListener(String protocol,
			 												  String serverUrl)
    {
    	if( protocol.equals("xmlrpc"))
    	{
    		UnifiedPresenceChangeListener changeListener = null;
    		try
    		{
    			if( (changeListener = changeListenerMap.get( serverUrl ) ) == null )
    			{
    				changeListener = new OpenfireXmlRcpUnifiedPresenceNotificationClient( serverUrl, sipXopenfirePlugin );
                    pushAllToListener( changeListener );
	    			changeListenerMap.put( serverUrl, changeListener );
        			log.debug("changeListenerSet adding server for URL" + serverUrl );
        		}
        		else
        		{
        			pushAllToListener( changeListener );
        			log.debug("changeListenerSet already contains server for URL" + serverUrl );
        		}
    		}
    		catch( Exception ex )
    		{
    			log.error("addUnifiedPresenceChangeListener caught exception: " + ex.getMessage() );
    		}
    	}
    }
    
    synchronized public void removeUnifiedPresenceChangeListener(String protocol,
			  													 String serverUrl)
    {
    	if( protocol.equals("xmlrpc"))
    	{
    		changeListenerMap.remove( serverUrl );
    	}
    }
    
    synchronized void notifyListenersOfChange( UnifiedPresence changedPresence )
    {
    	Collection<UnifiedPresenceChangeListener> listeners = changeListenerMap.values();
        for( UnifiedPresenceChangeListener listener : listeners )
        {
            listener.unifiedPresenceChanged( changedPresence.getXmppUsername(), changedPresence );
        }
    }
    
    synchronized void pushAllToListener( UnifiedPresenceChangeListener listenerToRefresh )
    {
        Collection<UnifiedPresence> unifiedPresences = unifiedPresenceMap.values();
        for( UnifiedPresence up : unifiedPresences )
        {
            listenerToRefresh.unifiedPresenceChanged( up.getXmppUsername(), up );
        }
    }

}
