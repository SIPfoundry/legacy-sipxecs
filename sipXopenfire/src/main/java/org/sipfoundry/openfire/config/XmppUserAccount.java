package org.sipfoundry.openfire.config;


public class XmppUserAccount  {
    
    private String userName;
    
    private String sipUserName;
    
    private String displayName;
    
    private String password;
    
    private String onThePhoneMessage;
    
    
    
    public XmppUserAccount () {
    }
    public String getEmail() {
        return null;
    }
    
    public String getDisplayName() {
        return this.displayName;
    }
    
    public String getPassword() {
        return this.password;
    }


    /**
     * @param password the password to set
     */
    public void setPassword(String password) {
        this.password = password;
    }
    /**
     * @param userName the userName to set
     */
    public void setUserName(String userName) {
        this.userName = userName;
    }
    /**
     * @return the userName
     */
    public String getUserName() {
        return userName;
    }
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
     * @param onThePhoneMessage the onThePhoneMessage to set
     */
    public void setOnThePhoneMessage(String onThePhoneMessage) {
        this.onThePhoneMessage = onThePhoneMessage;
    }
    /**
     * @return the onThePhoneMessage
     */
    public String getOnThePhoneMessage() {
        return onThePhoneMessage;
    }
    /**
     * @param displayName the displayName to set
     */
    public void setDisplayName(String displayName) {
        this.displayName = displayName;
    }
    

}
