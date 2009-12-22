package org.sipfoundry.siptester;

import java.util.HashSet;
import java.util.Iterator;

public class SipTesterConfig {
    
    private String logLevel;
 
    private String sipxProxyDomain;
    
    private String testerIpAddress;
    
    private int testerBasePort ;
    
    private static int allocatedPort ;
    
    private HashSet<String> testUsers = new HashSet<String>();

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

    /**
     * @param testerBasePort the testerBasePort to set
     */
    public void setTesterBasePort(String testerBasePort) {
        this.testerBasePort = Integer.parseInt(testerBasePort);
        allocatedPort = this.testerBasePort;
    }

    /**
     * @return the testerBasePort
     */
    public int getTesterBasePort() {
        return testerBasePort;
    }
    
    public void addTestUser(String userName) {
        this.testUsers.add(userName);
    }
    
    public HashSet<String> getTestUsers() {
        return this.testUsers;
        
    }
   
    
    public static int getPort() {
        return allocatedPort ++;
    }
    
    

}
