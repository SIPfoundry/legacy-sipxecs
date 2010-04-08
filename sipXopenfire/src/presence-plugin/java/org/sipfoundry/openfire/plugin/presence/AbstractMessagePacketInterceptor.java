package org.sipfoundry.openfire.plugin.presence;

import org.jivesoftware.openfire.interceptor.PacketInterceptor;

import org.xmpp.packet.Packet;
import org.jivesoftware.openfire.interceptor.PacketRejectedException;
import org.jivesoftware.openfire.session.Session;

public abstract class AbstractMessagePacketInterceptor implements PacketInterceptor
{
    
    public abstract void start(SipXOpenfirePlugin plugin);
    
    public void interceptPacket(Packet packet, Session session, boolean incoming,
            boolean processed) throws PacketRejectedException
    {

    }
    
}
