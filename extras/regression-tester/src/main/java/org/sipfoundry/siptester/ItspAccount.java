package org.sipfoundry.siptester;

import java.text.ParseException;
import java.util.List;
import java.util.ListIterator;

import gov.nist.javax.sip.clientauthutils.UserCredentials;

import javax.sip.header.UserAgentHeader;

public class ItspAccount implements UserCredentials {
    
    private String userName;
    private String password;
    private String itspProxyAddress;
    private int    itspProxyPort;
    private String itspProxyDomain;

    public ItspAccount () {
     
    }
    public String getItspProxyAddress() {
        return this.itspProxyAddress;
    }
    public int getItspProxyPort() {
        return this.itspProxyPort;
    }
    public void setItspProxyDomain(String proxyDomain) {
        this.itspProxyDomain = proxyDomain;
    }
    public String getItspProxyDomain() {
        return this.itspProxyDomain;
    }
    public void setItspProxyAddress( String proxyAddress ) {
        this.itspProxyAddress = proxyAddress;
    }
    
    public void setItspProxyPort(String proxyPort ) {
        this.itspProxyPort = Integer.parseInt(proxyPort);
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
     * @return the sipDomain
     */
    public String getSipDomain() {
        return SipTester.getTesterConfig().getTesterIpAddress();
    }

    /**
     * @param password the password to set
     */
    public void setPassword(String password) {
        this.password = password;
    }

    /**
     * @return the password
     */
    public String getPassword() {
        return password;
    }

    
  
}
