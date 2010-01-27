package org.sipfoundry.siptester;

import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;

public class SipTesterConfig {
    
    private String logLevel;
 
    private String sipxProxyDomain;
    
    private String testerIpAddress;
    
    private int basePort;

    private int rtpBasePort;
    
    private Hashtable<Integer,Integer> portMap = new Hashtable<Integer,Integer>();
      
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
    
    public void setRtpBasePort(String basePort) {
        this.rtpBasePort = Integer.parseInt(basePort);
    }
    
    public int getNextRtpPort() {
        int retval = this.rtpBasePort;
        rtpBasePort ++;
        return retval;
    }

    public int getMediaPort(int port) {
       if ( portMap.get(port) != null ) {
           return portMap.get(port);
       } else {
           int retval = getNextRtpPort();
           portMap.put(port, retval);
           return retval;
       }
    }

   

}
