package org.sipfoundry.openfire.config;

import org.apache.log4j.Logger;

public class XmppGroupMember {
    private static Logger logger = Logger.getLogger(XmppGroupMember.class);    
    private String jid = ""; 
    // NOTE: extend the equals() method if new instance variables get added
    
    public void setUserName(String xmppUserName) {
        this.jid = XmppAccountInfo.appendDomain(xmppUserName);
        this.jid = this.jid.toLowerCase();  // openfire fails to add users with mixed case
    }
    /**
     * @return the userName
     */
    public String getJid() {
        return jid;
    }
    
    @Override 
    public boolean equals(Object other) {
        //check for self-comparison
        if ( this == other ) return true;

        if ( !(other instanceof XmppGroupMember) ) return false;

        XmppGroupMember otherGroupMember = (XmppGroupMember)other;
        return jid.equals( otherGroupMember.jid );
    }
    
    @Override
    public int hashCode()
    {
        return jid.hashCode();
    }
    
    @Override
    public String toString()
    {
        return new StringBuilder("XmppGroupMember='")
        .append(this.getJid()).toString();     
    }
}
