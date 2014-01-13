package org.sipfoundry.openfire.plugin.presence;

import org.jivesoftware.openfire.interceptor.PacketInterceptor;

public abstract class AbstractMessagePacketInterceptor implements PacketInterceptor {

    public abstract void start(SipXOpenfirePlugin plugin);
}
