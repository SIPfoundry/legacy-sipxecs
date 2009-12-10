package org.sipfoundry.openfire.plugin.presence;

public class UserAccount {
    
    private String sipUserName;
    
    private String xmppUserName;

    /**
     * @param sipUserName the sipUserName to set
     */
    public void setSipUserName(String sipUserName) {
        this.sipUserName = sipUserName;
    }

    /**
     * @return the sipUserName
     */
    public String getSipUserName() {
        return sipUserName;
    }

    /**
     * @param xmppUserName the xmppUserName to set
     */
    public void setXmppUserName(String xmppUserName) {
        this.xmppUserName = xmppUserName;
    }

    /**
     * @return the xmppUserName
     */
    public String getXmppUserName() {
        return xmppUserName;
    }

}
