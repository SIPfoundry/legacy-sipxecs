/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileReader;
import java.net.InetAddress;
import java.util.HashSet;
import java.util.Properties;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.log4j.SipFoundryLayout;

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
    private String logLevel = "WARN";
    private String musicOnHoldName = "~~mh~";
    private boolean musicOnHoldEnabled = false;
    private int xmlRpcPort = 0;
    private int sipKeepalive = 20 ; // Seconds for SIP keepalive.
    private int mediaKeepalive = 100; // milisec for media keepalive.
    private String logFileDirectory = "/var/log/sipxpbx/";
    private int globalAddressRediscoveryPeriod = 30;
    private boolean reInviteSupported = true;
    private String autoAttendantName = null;
    private String symmitronHost;
    private int symmitronXmlRpcPort = 9090;
    private String sipxProxyTransport = "udp";
    private String sipxbridgeUserName = null;
    private String sipxbridgePassword = null;
    private int musicOnHoldDelayMiliseconds = 500;
    private HashSet<Integer> parkServerCodecs = new HashSet<Integer>();
    private boolean isSecure = true;
    private String sipxSupervisorHost;
    private int sipxSupervisorXmlRpcPort;
    private int callLimit = -1;
    
    private static Logger logger = Logger.getLogger(BridgeConfiguration.class);

    public BridgeConfiguration() {
        parkServerCodecs.add(RtpPayloadTypes.getPayloadType("PCMU"));
        parkServerCodecs.add(RtpPayloadTypes.getPayloadType("PCMA"));
        parkServerCodecs.add(RtpPayloadTypes.getPayloadType("G722"));
        parkServerCodecs.add(RtpPayloadTypes.getPayloadType("L16"));

    }

    /**
     * @param externalAddress the externalAddress to set
     */
    public void setExternalAddress(String externalAddress) {
        try {
            this.externalAddress = InetAddress.getByName(externalAddress).getHostAddress();
            if (this.localAddress == null) {
                this.setLocalAddress(externalAddress);
            }
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
     * Set the logLevel to reflect the sipXecs-defined level requested,
     * unless overridden in the log4j.properties file.
     * @param logLevel the logLevel to set (one of the sipXecs-defined levels)
     */
    public void setLogLevel(String level) {
        this.logLevel = SipFoundryLayout.mapSipFoundry2log4j(level).toString();  
        if (logLevel.equals("DEBUG")) {
            try {
                String log4jProps = Gateway.configurationPath + "/log4j.properties";
                if (new File(log4jProps).exists()) {
                    /*
                     * Override the file configuration setting.
                     */
                    Properties props = new Properties();
                    props.load(new FileInputStream(log4jProps));
                    String newLevel = props.getProperty("log4j.category.org.sipfoundry.sipxbridge");
                    if (newLevel != null) {
                        this.logLevel = newLevel;
                    }
                }
            } catch (Exception ex) {
                /* Ignore */
            }
        }
    }

    /**
     * @return the logLevel (a log4j level)
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

    public void setMusicOnHoldName(String musicOnHoldName) {
        this.musicOnHoldName = musicOnHoldName;
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
    public void setGlobalPort(String globalPort) {
      try {
        this.globalPort = Integer.parseInt(globalPort);
      } catch ( NumberFormatException ex) {
    	  logger.error("Bad global port specified. Check configuration file",ex);
      }
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
     * Whether or not to forward a REFER ( if the ITSP capablity allows us to do so). This feature
     * is turned off until further testing can be done.
     *
     * @return
     */
    public boolean isReferForwarded() {
        return false;
    }

    public void setSecure(boolean isSecure) {
        this.isSecure = isSecure;
    }

    public boolean isSecure() {
        return this.isSecure;
    }

    /**
     * @param sipXbridgeUserName the sipXbridgeUserName to set
     */
    public void setSipxbridgeUserName(String sipXbridgeUserName) {
        this.sipxbridgeUserName = sipXbridgeUserName;
    }

    /**
     * @return the sipXbridgeUserName
     */
    String getSipxbridgeUserName() {
        return sipxbridgeUserName;
    }

    /**
     * @param sipXbridgePassword the sipXbridgePassword to set
     */
    public void setSipxbridgePassword(String sipXbridgePassword) {
        this.sipxbridgePassword = sipXbridgePassword;
    }

    /**
     * @return the sipXbridgePassword
     */
    public String getSipxbridgePassword() {
        return sipxbridgePassword;
    }

    /**
     * @param musicOnHoldDelay the musicOnHoldDelay to set
     */
    public void setMusicOnHoldDelayMiliseconds(int musicOnHoldDelay) {
        this.musicOnHoldDelayMiliseconds = musicOnHoldDelay;
    }

    /**
     * @return the musicOnHoldDelay
     */
    public int getMusicOnHoldDelayMiliseconds() {
        return musicOnHoldDelayMiliseconds;
    }

    public void setMohSupportedCodecs(String mohCodecs) throws IllegalArgumentException {
        String[] codecs = mohCodecs.split("\\,");
        for (String codec : codecs) {
            if (!RtpPayloadTypes.isPayload(codec)) {
                throw new IllegalArgumentException("Could not find codec " + codec);
            }
            int codecNumber = RtpPayloadTypes.getPayloadType(codec);
            this.parkServerCodecs.add(codecNumber);
        }
    }

    /**
     * @return the parkServerCodecs
     */
    public HashSet<Integer> getParkServerCodecs() {
        return parkServerCodecs;
    }




    public void setSipXSupervisorXmlRpcPort(int port) {
        if (port <= 0)
            throw new IllegalArgumentException("Port : " + port);
        this.sipxSupervisorXmlRpcPort = port;
    }

    public int getSipXSupervisorXmlRpcPort() {
        return this.sipxSupervisorXmlRpcPort;
    }

    public void setSipXSupervisorHost(String hostName) {
        this.sipxSupervisorHost = hostName;
    }

    public String getSipXSupervisorHost() {
        return this.sipxSupervisorHost;
    }

    /**
     * @param maxCalls the maxCalls to set
     */
    public void setCallLimit(int maxCalls) {
        this.callLimit = maxCalls;
    }

    /**
     * @return the maxCalls
     */
    public int getCallLimit() {
        return callLimit;
    }

  
}
