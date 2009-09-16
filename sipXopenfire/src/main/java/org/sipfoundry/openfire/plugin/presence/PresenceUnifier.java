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
 * SIP user linked to openfire users.  Any sub-component of this plug-in
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
        if (unifiedPresence == null){
            unifiedPresence = new UnifiedPresence( xmppUsername );
            unifiedPresenceMap.put( xmppUsername,  unifiedPresence );
        }
        unifiedPresence.setSipState( newState );
        log.debug("SipStateChanged for " + xmppUsername + " new state: " + newState + " new unified presence: " + unifiedPresence.getUnifiedPresence() );
        // generate new status message that contains SIP state information
        if (newState == SipResourceState.BUSY){
            // on the phone - craft new XMPP status message that represents that.
            String newXmppStatusMessageWithSipState = 
                generateXmppStatusMessageWithSipState( unifiedPresence.getJidAsString(), unifiedPresence );
            if( newXmppStatusMessageWithSipState != null ){
                unifiedPresence.setXmppStatusMessageWithSipState(newXmppStatusMessageWithSipState);
                // make plug-in aware of change so that it gets broadcasted to all watchers.
            }
            else{
                // no stauts message embedding 'on the phone' info generated, just default to the
                // plain XMPP status message then.
                unifiedPresence.setXmppStatusMessageWithSipState(unifiedPresence.getXmppStatusMessage());       
            }
        }
        else{
            // not on the phone. Set XMPP status message to reflect that.
            unifiedPresence.setXmppStatusMessageWithSipState(unifiedPresence.getXmppStatusMessage());
        }
        // make plug-in aware of change so that it gets broadcasted to all watchers.
        try{
            sipXopenfirePlugin.setPresenceStatus(unifiedPresence.getJidAsString(), unifiedPresence.getXmppStatusMessageWithSipState());
        }
        catch( Exception e ){
            log.error("PresenceUnifier::sipStateChanged() - caught exception for user " + unifiedPresence.getJidAsString() + ":" + e );
        }
        notifyListenersOfChange( unifiedPresence );
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
        log.debug("XmppStatusMessageChanged for " + xmppUsername + " new XMPP status message: " + newXmppStatusMessage );
        // check to see if this message is the echo of us adding the SIP state to the XMPP status message.
        // If it is, just skip it as the notification does not contain any info we don't already know.
        if( !unifiedPresence.getXmppStatusMessageWithSipState().equals(newXmppStatusMessage) ){
            // this is a genuine XMPP status message change from the user - add the SIP State 
            // information if we are on a call.
            unifiedPresence.setXmppStatusMessage( newXmppStatusMessage );
            if ( unifiedPresence.getSipState() == SipResourceState.BUSY ){
                String newXmppStatusMessageWithSipState = 
                    generateXmppStatusMessageWithSipState( unifiedPresence.getJidAsString(), unifiedPresence );
                if( newXmppStatusMessageWithSipState != null ){
                    unifiedPresence.setXmppStatusMessageWithSipState(newXmppStatusMessageWithSipState);
                }
                else{
                    unifiedPresence.setXmppStatusMessageWithSipState(unifiedPresence.getXmppStatusMessage());
                }
            }
            else{
                // not on the phone. Set XMPP status message to reflect that.
                unifiedPresence.setXmppStatusMessageWithSipState(unifiedPresence.getXmppStatusMessage());
            }
            try{
                sipXopenfirePlugin.setPresenceStatus(unifiedPresence.getJidAsString(), unifiedPresence.getXmppStatusMessageWithSipState());                
            }
            catch( Exception e ){
                log.error("PresenceUnifier::xmppStatusMessageChanged() - caught exception for user " + unifiedPresence.getJidAsString() + ":" + e );
            }
            notifyListenersOfChange( unifiedPresence );
        }
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

  //@returns: null if the user does not have an 'on the phone' message
    private String generateXmppStatusMessageWithSipState( String jid, UnifiedPresence unifiedPresence ){
        String xmppStatusMessageWithSipState = null;
        try{
            xmppStatusMessageWithSipState = sipXopenfirePlugin.getOnThePhoneMessage(unifiedPresence.getJidAsString());
            if (xmppStatusMessageWithSipState != null){
                if (unifiedPresence.getXmppStatusMessage().length() != 0 ){
                    xmppStatusMessageWithSipState = xmppStatusMessageWithSipState + " - " + unifiedPresence.getXmppStatusMessage();
                }
            }
        }
        catch( Exception e ){
            log.error("PresenceUnifier::generateXmppStatusMessageWithSipState() - caught exception for user " + unifiedPresence.getJidAsString() + ":" + e );
        }            
        return xmppStatusMessageWithSipState;
    }
}


