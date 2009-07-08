package org.sipfoundry.openfire.plugin.presence;

import org.xmpp.packet.Presence;

public class PresenceState {
    public static final String ETENDED_AWAY = Presence.Show.xa.toString();
    
    public static final String BUSY = Presence.Show.dnd.toString();
    
    public static final String AWAY = Presence.Show.away.toString();
    
    public static final String CHAT = Presence.Show.chat.toString();
    
    public static final String ONLINE = "available";
    
    public static final String OFFLINE = "offline";

}
