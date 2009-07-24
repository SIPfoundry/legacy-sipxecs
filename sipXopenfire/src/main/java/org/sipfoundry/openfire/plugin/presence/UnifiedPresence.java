package org.sipfoundry.openfire.plugin.presence;

import org.sipfoundry.sipcallwatcher.SipResourceState;
import org.xmpp.packet.Presence;

public class UnifiedPresence {
    
    public enum XmppPresence
    {
        OFFLINE(null),
        AVAILABLE(null),
        CHAT(Presence.Show.chat),
        AWAY(Presence.Show.away),
        EXTENDED_AWAY(Presence.Show.xa),
        BUSY(Presence.Show.dnd);
        
        private XmppPresence( Presence.Show correspondingPresenceShowEnum )
        {
            this.correspondingPresenceShowEnum = correspondingPresenceShowEnum;
        }
        
        public Presence.Show asPresenceShowEnum()
        {
            return this.correspondingPresenceShowEnum;
        }
        
        private Presence.Show correspondingPresenceShowEnum;
    }

    
    public static final String NOT_AVAILABLE = "not-available";
    public static final String AVAILABLE_FOR_PHONE = "available-for-phone";
    public static final String AVAILABLE_FOR_CHAT = "available-for-chat";
    public static final String AVAILABLE_FOR_BOTH = "available-for-both";

    private String unifiedPresence = NOT_AVAILABLE;
    private SipResourceState sipState = SipResourceState.UNDETERMINED;
    private XmppPresence xmppPresence = XmppPresence.OFFLINE;
    private String xmppStatusMessage = ""; 
    private String xmppUsername; // XMPP username that presence info relates to
    
    UnifiedPresence( String xmppUsername )
    {
        xmppPresence = XmppPresence.OFFLINE;
        this.xmppUsername = xmppUsername;
    }
    
    public SipResourceState getSipState()
    {
        return sipState;
    }
    
    public void setSipState( SipResourceState sipState )
    {
        this.sipState = sipState;
        computeUnifiedPresence();
    }
    
    public XmppPresence getXmppPresence()
    {
        return xmppPresence;
    }

    public void setXmppPresence( XmppPresence newXmppPresence )
    {
        this.xmppPresence = newXmppPresence;
    }

    public String getXmppStatusMessage()
    {
        return this.xmppStatusMessage;
    }

    public void setXmppStatusMessage( String newXmppStatusMessage )
    {
        this.xmppStatusMessage = newXmppStatusMessage;
    }
    
    public void setXmppPresence( Presence presence )
    {
        if( !presence.isAvailable() )
        {
            this.xmppPresence = XmppPresence.OFFLINE;
        }
        else
        {
            if( presence.getShow() == null )
            {
                this.xmppPresence = XmppPresence.AVAILABLE;
            }
            else if( presence.getShow() == Presence.Show.away )
            {
                this.xmppPresence = XmppPresence.AWAY;                
            }
            else if( presence.getShow() == Presence.Show.dnd )
            {
                this.xmppPresence = XmppPresence.BUSY;                
            }
            else if( presence.getShow() == Presence.Show.xa )
            {
                this.xmppPresence = XmppPresence.EXTENDED_AWAY;                
            }
            else if( presence.getShow() == Presence.Show.chat )
            {
                this.xmppPresence = XmppPresence.CHAT;                
            }
            else
            {
                this.xmppPresence = XmppPresence.OFFLINE;                
            }
        }
        setXmppStatusMessage( presence.getStatus() );
        computeUnifiedPresence();
    }
    
    public String getUnifiedPresence()
    {
        return unifiedPresence;
    }
    
    private void computeUnifiedPresence()
    {
        boolean bAvailableForChat = false;
        boolean bAvailableForTelephony = false;
        
        if( xmppPresence == XmppPresence.AVAILABLE ||
            xmppPresence == XmppPresence.CHAT )
        {
            bAvailableForChat = true;
        }
             
        if( sipState == SipResourceState.IDLE )
        {
            bAvailableForTelephony = true;
        }
        
        // map the bAvailableForChat and bAvailableForTelephony to a unified presence
        if( bAvailableForChat )
        {
            if( bAvailableForTelephony )
            {
                unifiedPresence = AVAILABLE_FOR_BOTH;
            }
            else
            {
                unifiedPresence = AVAILABLE_FOR_CHAT;
            }
        }
        else
        {
            if( bAvailableForTelephony )
            {
                unifiedPresence = AVAILABLE_FOR_PHONE;
            }
            else
            {
                unifiedPresence = NOT_AVAILABLE;
            }
        }
    }

    public String getXmppUsername()
    {
        return xmppUsername;
    }
}
