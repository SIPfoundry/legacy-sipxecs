/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.config;

import org.jivesoftware.openfire.user.UserNotFoundException;
import org.sipfoundry.openfire.plugin.presence.SipXOpenfirePlugin;

public class XmppUserAccount extends XmppConfigurationElement {

    private String userName = "";

    private String sipUserName = "";

    private String displayName = "";

    private String password = "";

    private String email = "";

    private String onThePhoneMessage = "";

    private boolean bAdvertiseOnCallStatus = false;

    private boolean bShowOnCallDetails = false;
    // NOTE: extend the equals() method if new instance variables get added

    public XmppUserAccount () {
    }

    public String getEmail() {
        return email;
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
     * @param email
     */
    public void setEmail(String email) {
        this.email = email;
    }

    /**
     * @param userName the userName to set
     */
    public void setUserName(String userName) {
        this.userName = userName.toLowerCase();  // Necessary because openfire stores usernames in lowercase.
        // We force username to lowercase to ensure that compares
        // between this username and ones taken from the openfire DB
        // work as expected.
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

    public void setAdvertiseOnCallPreference(String flag){
        this.bAdvertiseOnCallStatus = Boolean.parseBoolean(flag);
    }

    public boolean getAdvertiseOnCallPreference(){
        return this.bAdvertiseOnCallStatus;
    }

    public void setShowOnCallDetailsPreference(String flag){
        this.bShowOnCallDetails = Boolean.parseBoolean(flag);
    }

    public boolean getShowOnCallDetailsPreference(){
        return this.bShowOnCallDetails;
    }

    @Override
    public void update() throws UserNotFoundException {
        SipXOpenfirePlugin.getInstance().update(this);
    }

    @Override
    public boolean equals(Object other) {
        //check for self-comparison
        if ( this == other ) {
            return true;
        }

        if ( !(other instanceof XmppUserAccount) ) {
            return false;
        }

        //cast to native object is now safe
        XmppUserAccount otherAccount = (XmppUserAccount)other;

        //now a proper field-by-field evaluation can be made
        return
                userName.equals(otherAccount.userName)                        &&
                sipUserName.equals(otherAccount.sipUserName)                  &&
                displayName.equals(otherAccount.displayName)                  &&
                password.equals(otherAccount.password)                        &&
                email.equals(otherAccount.getEmail())                         &&
                onThePhoneMessage.equals(otherAccount.onThePhoneMessage)      &&
                bAdvertiseOnCallStatus == otherAccount.bAdvertiseOnCallStatus &&
                bShowOnCallDetails == otherAccount.bShowOnCallDetails;
    }

    @Override
    public int hashCode()
    {
        return userName.hashCode();
    }

    @Override
    public String toString()
    {
        return new StringBuilder("XmppUserAccount='")
        .append("'\n    name='")
        .append(this.getUserName())
        .append("'\n    status='")
        .append(this.getStatus())
        .append("'\n    display name='")
        .append(this.getDisplayName())
        .append("'\n    email='")
        .append(this.getEmail())
        .append("'\n    advertiseOnCall='")
        .append(this.getAdvertiseOnCallPreference())
        .append("'\n    onthephoneMessage='")
        .append(this.getOnThePhoneMessage())
        .append("'\n    password='")
        .append(this.getPassword())
        .append("'\n    onCallDetailsPref='")
        .append(this.getShowOnCallDetailsPreference())
        .append("'\n    SIP user name='")
        .append(this.getSipUserName())
        .append("'\n===============\n").toString();
    }
}
