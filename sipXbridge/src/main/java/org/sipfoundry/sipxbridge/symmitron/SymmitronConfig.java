/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge.symmitron;

import java.net.InetAddress;

public class SymmitronConfig {
    
    private int portRangeLowerBound = 30000;
    private int portRangeUpperBound = 30500;
    private int xmlRpcPort = 8081;
    private String localAddress;
    private String externalAddress;
    private String logFileName = null;
    private String logFileDirectory = "/var/log/sipxbpx";
    private String logLevel = "INFO";
   
    
    
    public SymmitronConfig() {
        
    }
    
    public void setXmlRpcPort ( int xmlRpcPort ) {
        this.xmlRpcPort = xmlRpcPort;
    }
    /**
     * @param portRange
     */
    public void setPortRange(String portRange) {
        String[] ports = portRange.split(":");
        if (ports.length != 2) {
            throw new IllegalArgumentException(
                    "Must have format lower:upper bound");
        }
        String lowBound = ports[0];
        String highBound = ports[1];
        this.portRangeLowerBound = new Integer(lowBound).intValue();
        this.portRangeUpperBound = new Integer(highBound).intValue();
        if (this.portRangeLowerBound >= this.portRangeUpperBound
                || this.portRangeLowerBound < 0 || this.portRangeUpperBound < 0) {
            throw new IllegalArgumentException(
                    "Port range should be lower:upper bound integers and postivie");
        }
    }
    
    /**
     * @param localAddress
     *            the localAddress to set
     */
    public void setLocalAddress(String localAddress) {
        try {
            this.localAddress = InetAddress.getByName(localAddress)
                    .getHostAddress();
        } catch (Exception ex) {
            throw new IllegalArgumentException("invalid address : "
                    + localAddress);
        }
    }
    
  
    /**
     * @return the xmlRpcPort
     */
    public int getXmlRpcPort() {
        return xmlRpcPort;
    }

    /**
     * @return the localAddress
     */
    public String getLocalAddress() {
        return localAddress;
    }

    public int getPortRangeLowerBound() {
        return portRangeLowerBound;
    }

    /**
     * @param portRangeLowerBound the portRangeLowerBound to set
     */
    public void setPortRangeLowerBound(int portRangeLowerBound) {
        this.portRangeLowerBound = portRangeLowerBound;
    }

    /**
     * @param portRangeUpperBound the portRangeUpperBound to set
     */
    public void setPortRangeUpperBound(int portRangeUpperBound) {
        this.portRangeUpperBound = portRangeUpperBound;
    }

    /**
     * @return the portRangeUpperBound
     */
    public int getPortRangeUpperBound() {
        return portRangeUpperBound;
    }

    public String getExternalAddress() {
        return externalAddress;
    }
    
    public void setExternalAddress ( String externalAddress ) {
        this.externalAddress = externalAddress;
    }


   
    public void setLogFileDirectory(String logFileDirectory) {
        this.logFileDirectory = logFileDirectory;
    }

  
    public String getLogFileDirectory() {
        return logFileDirectory;
    }

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
     * @param logFileName the logFileName to set
     */
    public void setLogFileName(String logFileName) {
        this.logFileName = logFileName;
    }

    /**
     * @return the logFileName
     */
    public String getLogFileName() {
        return logFileName;
    }
   
}
