package org.sipfoundry.openfire.config;

public class XmppGroupMember {
    private String userName;
    
    public void setUserName(String userName) {
        this.userName = XmppAccountInfo.appendDomain(userName);
    }
    /**
     * @return the userName
     */
    public String getUserName() {
        return userName;
    }
    
}
