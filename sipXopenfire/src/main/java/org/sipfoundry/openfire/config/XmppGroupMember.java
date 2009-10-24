package org.sipfoundry.openfire.config;

import org.apache.log4j.Logger;

public class XmppGroupMember {
    private static Logger logger = Logger.getLogger(XmppGroupMember.class);    
    private String jid; 
    
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
    
}
