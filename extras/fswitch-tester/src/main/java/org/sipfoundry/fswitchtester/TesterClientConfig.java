/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.fswitchtester;

import org.apache.xmlrpc.client.XmlRpcClient;

public class TesterClientConfig {
    private int userAgentCount;
    private String testerHost;
    private int clientId;
    private int testerSipBasePort;
    private int xmlRpcPort;
    private String mediaFile;
    private String conferenceExtension;
    
    
    private int testerRtpBasePort;
    private XmlRpcClient xmlRpcClient;
    private int senderCount;
    
    private String mediaFileName;
  
    /**
     * @param conferenceSize the conferenceSize to set
     */
    public void setUserAgentCount(int conferenceSize) {
        this.userAgentCount = conferenceSize;
    }
    
    /**
     * @return the conferenceSize
     */
    public int getUserAgentCount() {
        return userAgentCount;
    }
    
    /**
     * @param testerHost the testerHost to set
     */
    public void setTesterHost(String testerHost) {
        this.testerHost = testerHost;
    }
    
    /**
     * @return the testerHost
     */
    public String getTesterHost() {
        return testerHost;
    }
    
    /**
     * @param clientId the clientId to set
     */
    public void setClientId(int clientId) {
        this.clientId = clientId;
    }
    
    /**
     * @return the clientId
     */
    public int getClientId() {
        return clientId;
    }
    public void setTesterSipBasePort( int basePort) {
        this.testerSipBasePort = basePort;
    }
    
    public void setTesterRtpBasePort( int basePort) {
        this.testerRtpBasePort = basePort;
    }
    
    /**
     * @return the testerSipBasePort
     */
    public int getTesterSipBasePort() {
        return testerSipBasePort;
    }
    
    /**
     * @return the testerRtpBasePort
     */
    public int getTesterRtpBasePort() {
        return testerRtpBasePort;
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
    public void setXmlRpcClient(XmlRpcClient client) {
        this.xmlRpcClient = client;
        
    }
    
    public XmlRpcClient getXmlRpcClient() {
        return this.xmlRpcClient;
    }

    /**
     * @param mediaFile the mediaFile to set
     */
    public void setMediaFile(String mediaFile) {
        this.mediaFile = mediaFile;
    }

    /**
     * @return the mediaFile
     */
    public String getMediaFile() {
        return mediaFile;
    }
    
    public void setSenderCount(int senderCount) {
        this.senderCount = senderCount;
    }
    
    public int getSenderCount() {
        return this.senderCount;
    }

    public void setConferenceExtension( String conferenceExtension ) {
        this.conferenceExtension = conferenceExtension;
    }
    public String getConferenceExtension() {
        return this.conferenceExtension;
    }

    
}
