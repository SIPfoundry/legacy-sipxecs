package org.sipfoundry.openfire.plugin.presence;

public interface UnifiedPresenceChangeListener
{
    public void unifiedPresenceChanged( String xmppUsername, UnifiedPresence newUnifiedPresence );
}
