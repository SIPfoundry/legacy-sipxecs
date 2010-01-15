package org.sipfoundry.siptester;

import java.util.HashSet;
import java.util.Iterator;

public class SipTesterConfig {
    
    private String logLevel;
 
    private String sipxProxyDomain;
    
    private String testerIpAddress;
    
    private int basePort;
      
    /**
     * @param logLevel the logLevel to set
     */
    public void setLogLevel(String logLevel) {
        this.logLevel = logLevel;
    }

    /**
     * @return the logLevel
     */
    public String getLogLevel() {
        return logLevel;
    }

   
    /**
     * @param proxyDomain the proxyDomain to set
     */
    public void setSipxProxyDomain(String proxyDomain) {
        this.sipxProxyDomain = proxyDomain;
    }

    /**
     * @return the proxyDomain
     */
    public String getSipxProxyDomain() {
        return sipxProxyDomain;
    }

    /**
     * @param testerIpAddress the testerIpAddress to set
     */
    public void setTesterIpAddress(String testerIpAddress) {
        this.testerIpAddress = testerIpAddress;
    }

    /**
     * @return the testerIpAddress
     */
    public String getTesterIpAddress() {
        return testerIpAddress;
    }
    
    public void setBasePort(String basePort) {
        this.basePort = Integer.parseInt(basePort);
    }
    
    public int getBasePort() {
        return this.basePort;
    }

   

}
