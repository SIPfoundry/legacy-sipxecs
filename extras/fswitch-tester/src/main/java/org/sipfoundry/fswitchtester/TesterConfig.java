/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.fswitchtester;

import java.util.Collection;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.LinkedList;
import java.util.List;

public class TesterConfig {

    private String sipxProxyDomain;
    private String sipxProxyAddress;
     private int clientId;
    private int testDuration;

    private int sipxProxyPort = 5060;
    private String codec;
    private Hashtable<Integer, TesterClientConfig> testerClients = new Hashtable<Integer, TesterClientConfig>();
    private List<Integer> clientIds = new LinkedList<Integer> ();
    
    
   

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
     * @param sipxProxyAddress the sipxProxyAddress to set
     */
    public void setSipxProxyAddress(String sipxProxyAddress) {
        this.sipxProxyAddress = sipxProxyAddress;
    }

    /**
     * @return the sipxProxyAddress
     */
    public String getSipxProxyAddress() {
        return sipxProxyAddress;
    }

    

    /**
     * @return the conferenceExtension
     */
    public String getConferenceExtension() {
        return this.getTesterClientConfig().getConferenceExtension();
    }

    /**
     * @param sipxProxyPort the sipxProxyPort to set
     */
    public void setSipxProxyPort(int sipxProxyPort) {
        this.sipxProxyPort = sipxProxyPort;
    }

    /**
     * @return the sipxProxyPort
     */
    public int getSipxProxyPort() {
        return sipxProxyPort;
    }

    /**
     * @param codec the codec to set
     */
    public void setCodec(String codec) {
        this.codec = codec;
    }

    /**
     * @return the codec
     */
    public String getCodec() {
        return codec;
    }

    /**
     * Add a test client.
     */
    public void addTestClient(TesterClientConfig testerClientConfig) {
        this.testerClients.put(testerClientConfig.getClientId(),testerClientConfig);
        this.clientIds.add(testerClientConfig.getClientId());
    }

    public void setClientId(String clientId) {
       this.clientId = Integer.parseInt(clientId);
    }
    
    public String getTesterHost() {
        return this.testerClients.get(clientId).getTesterHost();
    }
    
    public int getTesterSipBasePort() {
        return this.testerClients.get(clientId).getTesterSipBasePort();
    }
    
    public int getTesterRtpBasePort() {
        return this.testerClients.get(clientId).getTesterRtpBasePort();
    }
    
    public int getConferenceSize() {
        return this.testerClients.get(clientId).getUserAgentCount();
    }

    public TesterClientConfig getTesterClientConfig() {
        return this.testerClients.get(clientId);
       
    }
    
    public int getXmlRpcPort() {
        return this.getTesterClientConfig().getXmlRpcPort();
    }
    
   

    public Collection<TesterClientConfig> getTesterClients() {
     
        return this.testerClients.values();
    }

    public boolean isFirstClient() {
        return this.clientId == this.clientIds.get(0);
    }

    /**
     * @param testDuration the testDuration to set
     */
    public void setTestDuration(int testDuration) {
        this.testDuration = testDuration;
    }

    /**
     * @return the testDuration
     */
    public int getTestDuration() {
        return testDuration;
    }

    
}
