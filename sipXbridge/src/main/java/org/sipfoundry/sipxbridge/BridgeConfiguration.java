/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.net.InetAddress;

import org.apache.log4j.Logger;

/**
 * A class that represents the configuration of the SipxBridge. IMPORTANT -- the methods of this
 * class are tied to sipxbridge.xsd. Do not change method names or signatures unless you also edit
 * schema/sipxbridge.xsd.
 * 
 * 
 * @author M. Ranganathan
 * 
 */
public class BridgeConfiguration {
    private String globalAddress;
    private int globalPort = -1;
    private String externalAddress;
    private String localAddress;
    private int externalPort = 5080;
    private int localPort = 5090;
    private int sipxProxyPort = -1;
    private String sipxProxyDomain;
    private String stunServerAddress = null;
    private String logLevel = "INFO";
    private int rtpPortLowerBound = 25000;
    private int rtpPortUpperBound = 25500;
    private String musicOnHoldName = "~~mh~";
    private boolean musicOnHoldEnabled = false;
    private int xmlRpcPort = 0;
    private int sipKeepalive = 20 * 1000; // Miliseconds for SIP keepalive.
    private int mediaKeepalive = 100; // milisec for media keepalive.
    private String logFileDirectory = "/var/log/sipxpbx/";
    private int globalAddressRediscoveryPeriod = 30;
    private String codecName = "PCMU";
    private boolean reInviteSupported = true;
    private int maxCalls = -1;
    private String autoAttendantName = null;
    private String symmitronHost;
    private int symmitronXmlRpcPort = 0;
    private String sipxProxyTransport = "udp";
    
    private boolean isSecure = false;

    private static Logger logger = Logger.getLogger(BridgeConfiguration.class);

    /**
     * @param externalAddress the externalAddress to set
     */
    public void setExternalAddress(String externalAddress) {
        try {
            this.externalAddress = InetAddress.getByName(externalAddress).getHostAddress();
        } catch (Exception ex) {
            throw new IllegalArgumentException("invalid address : " + externalAddress);
        }
    }

    public boolean isInboundCallsRoutedToAutoAttendant() {
        return this.autoAttendantName != null;
    }

    public void setAutoAttendantName(String autoAttendantName) {
        this.autoAttendantName = autoAttendantName;
    }

    public String getAutoAttendantName() {
        return this.autoAttendantName;
    }

    /**
     * @return the externalAddress
     */
    public String getExternalAddress() {
        return externalAddress;
    }

    /**
     * @param localAddress the localAddress to set
     */
    public void setLocalAddress(String localAddress) {
        try {
            this.localAddress = InetAddress.getByName(localAddress).getHostAddress();
        } catch (Exception ex) {
            throw new IllegalArgumentException("invalid address : " + localAddress);
        }
    }

    /**
     * @return the localAddress
     */
    public String getLocalAddress() {
        return localAddress;
    }

    /**
     * @param externalPort the externalPort to set
     */
    public void setExternalPort(int externalPort) {
        this.externalPort = externalPort;
    }

    /**
     * @return the externalPort
     */
    public int getExternalPort() {
        return externalPort;
    }

    /**
     * @param localPort the localPort to set
     */
    public void setLocalPort(int localPort) {
        this.localPort = localPort;
    }

    /**
     * @return the localPort
     */
    public int getLocalPort() {
        return localPort;
    }

    /**
     * @param sipxProxyDomain the sipxProxyDomain to set
     */
    public void setSipxProxyDomain(String sipxProxyDomain) {
        this.sipxProxyDomain = sipxProxyDomain;
    }

    /**
     * @return the sipxProxyDomain
     */
    public String getSipxProxyDomain() {
        return sipxProxyDomain;
    }

    /**
     * Set the preferred transport of the sipx proxy.
     */
    public void setSipxProxyTransport(String transport) {
        this.sipxProxyTransport = transport;
    }

    /**
     * Get the sipx transport (if any has been set)
     */
    public String getSipxProxyTransport() {
        return this.sipxProxyTransport;
    }

    /**
     * @param stunServerAddress the stunServerAddress to set
     */
    public void setStunServerAddress(String stunServerAddress) {

        this.stunServerAddress = stunServerAddress;
    }

    /**
     * @return the stunServerAddress
     */
    public String getStunServerAddress() {
        return stunServerAddress;
    }

    /**
     * set the global address rediscovery period.
     */
    public void setGlobalAddressRediscoveryPeriod(String period) {
        try {
            this.globalAddressRediscoveryPeriod = Integer.parseInt(period);
        } catch (NumberFormatException ex) {
            logger.error("Illegal Format " + period);
        }
    }

    /**
     * @return the globalAddressRediscoveryPeriod
     */
    public int getGlobalAddressRediscoveryPeriod() {
        return globalAddressRediscoveryPeriod;
    }

    /**
     * @param logLevel the logLevel to set
     */
    public void setLogLevel(String logLevel) {
        this.logLevel = logLevel;
    }

    /**
     * @param portRange
     */
    public void setPortRange(String portRange) {
        String[] ports = portRange.split(":");
        if (ports.length != 2) {
            throw new IllegalArgumentException("Must have format lower:upper bound");
        }
        String lowBound = ports[0];
        String highBound = ports[1];
        this.rtpPortLowerBound = new Integer(lowBound).intValue();
        this.rtpPortUpperBound = new Integer(highBound).intValue();
        if (this.rtpPortLowerBound >= this.rtpPortUpperBound || this.rtpPortLowerBound < 0
                || this.rtpPortUpperBound < 0) {
            throw new IllegalArgumentException(
                    "Port range should be lower:upper bound integers and postivie");
        }
    }

    /**
     * @return the logLevel
     */
    public String getLogLevel() {
        return logLevel;
    }

    /**
     * @return Return the MOH UserName or null if no MOH is supported.
     */
    public String getMusicOnHoldName() {
        if (!this.isMusicOnHoldSupportEnabled())
            return null;
        else
            return this.musicOnHoldName;
    }

    /**
     * @param musicOnHoldEnabled the musicOnHoldEnabled to set
     */
    public void setMusicOnHoldSupportEnabled(boolean musicOnHoldEnabled) {
        this.musicOnHoldEnabled = musicOnHoldEnabled;
    }

    /**
     * @return the musicOnHoldEnabled
     */
    public boolean isMusicOnHoldSupportEnabled() {
        return musicOnHoldEnabled;
    }

    /**
     * @param xmlRpcPort the xmlRpcPort to set
     */
    public void setXmlRpcPort(int xmlRpcPort) {
        this.xmlRpcPort = xmlRpcPort;
    }

    /**
     * @return the xmlRpcPort
     */
    public int getXmlRpcPort() {
        return xmlRpcPort;
    }

    /**
     * @param sipKeepalive the sipKeepalive to set
     */
    public void setSipKeepalive(String sipKeepalive) {
        try {
            this.sipKeepalive = Integer.parseInt(sipKeepalive);
        } catch (NumberFormatException ex) {
            logger.error("Illegal Arg " + sipKeepalive);
        }
    }

    /**
     * @return the sipKeepalive
     */
    public int getSipKeepalive() {
        return sipKeepalive;
    }

    /**
     * @param mediaKeepalive the mediaKeepalive to set
     */
    public void setMediaKeepalive(String mediaKeepalive) {
        try {
            this.mediaKeepalive = Integer.parseInt(mediaKeepalive) * 1000;
        } catch (NumberFormatException ex) {
            logger.error("Illegal Arg " + mediaKeepalive);
        }
    }

    /**
     * @return the mediaKeepalive
     */
    public int getMediaKeepalive() {
        return mediaKeepalive;
    }

    /**
     * @param logFileName the logFileName to set
     */
    public void setLogFileDirectory(String logFileDirectory) {
        this.logFileDirectory = logFileDirectory;
    }

    /**
     * @return the logFileName
     */
    public String getLogFileDirectory() {
        return logFileDirectory;
    }

    public void setCodecName(String codecName) {
        this.codecName = codecName;
    }

    public String getCodecName() {
        return this.codecName;
    }

    /**
     * @param maxCalls the maxCalls to set
     */
    public void setMaxCalls(String maxCalls) {
        try {
            this.maxCalls = Integer.parseInt(maxCalls);
        } catch (NumberFormatException ex) {
            logger.error("Bad Param " + maxCalls);
        }
    }

    /**
     * @return the maxCalls
     */
    public int getMaxCalls() {
        return maxCalls;
    }

    public boolean isReInviteSupported() {
        return this.reInviteSupported;
    }

    /**
     * @param reInviteSupported the reInviteSupported to set
     */
    public void setReInviteSupported(boolean reInviteSupported) {
        this.reInviteSupported = reInviteSupported;
    }

    /**
     * @param globalAddress the globalAddress to set
     */
    public void setGlobalAddress(String globalAddress) {
        this.globalAddress = globalAddress;
    }

    /**
     * @return the globalAddress
     */
    public String getGlobalAddress() {
        return globalAddress;
    }

    /**
     * @param globalPort the globalPort to set
     */
    public void setGlobalPort(int globalPort) {
        this.globalPort = globalPort;
    }

    /**
     * @return the globalPort
     */
    public int getGlobalPort() {
        return globalPort;
    }

    public void setSymmitronHost(String symmitronAddress) {
        this.symmitronHost = symmitronAddress;
    }

    public String getSymmitronHost() {
        return symmitronHost;
    }

    /**
     * @param symmitronXmlRpcPort the symmitronXmlRpcPort to set
     */
    public void setSymmitronXmlRpcPort(String symmitronXmlRpcPort) {
        try {
            this.symmitronXmlRpcPort = Integer.parseInt(symmitronXmlRpcPort);
        } catch (NumberFormatException ex) {
            logger.error("Bad config parameter ", ex);
        }
    }

    /**
     * @return the symmitronXmlRpcPort
     */
    public int getSymmitronXmlRpcPort() {
        return symmitronXmlRpcPort;
    }

    /**
     * @param sipxProxyPort the sipxProxyPort to set
     */
    public void setSipxProxyPort(String sipxProxyPort) {
        try {
            int port = Integer.parseInt(sipxProxyPort);
            if (port < 0) {
                throw new NumberFormatException("Should be a positive integer");
            }
            this.sipxProxyPort = port;
        } catch (NumberFormatException ex) {
            logger.error("Illegal integer for sipx proxy port" + sipxProxyPort);
        }
    }

    /**
     * @return the sipxProxyPort
     */
    public int getSipxProxyPort() {
        return sipxProxyPort;
    }

    /**
     * Whether or not to forward a REFER ( if the ITSP capablity allows us to do so).
     * This feature is turned off until further testing can be done.
     * 
     * @return
     */
    public boolean isReferForwarded() {
        
        return false;
        
    }
    
    public void setSecure(boolean isSecure ) {
        this.isSecure = isSecure;
    }
    
    public boolean isSecure() {
        return this.isSecure;
    }

}
