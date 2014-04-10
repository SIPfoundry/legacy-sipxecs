/**
 *
 *
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxivr;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.log4j.SipFoundryLayout;


/**
 * Holds the configuration data needed for sipXivr.
 *
 */
public class SipxIvrConfiguration {

    private static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    
    private String m_logFile; // The file to log into
    private int m_eventSocketPort; // The Event Socket Listen port
    private String m_dataDirectory; // File path to the media server data directory
    private String m_mailstoreDirectory; // File path to the mailstore
    private String m_promptsDirectory; // File path to the AA prompts
    private String m_organizationPrefs; // File path to organizationprefs.xml
    private String m_scriptsDirectory; // File path to the AA scripts (for schedule access)
    private String m_docDirectory; // File path to DOC Directory (usually /usr/share/www/doc)
    private String m_operatorAddr; // Address of 'operator'
    private String m_sipxchangeDomainName; // The domain name of this system
    private String m_realm;
    private String m_mwiUrl; // The url of the Status Server we send MWI requests to
    private String m_configUrl; // The url of the Config Server for PIN change requests
    private int m_httpPort; // The port on which we listen for HTTP services
    private String m_sendIMUrl;
    private String m_openfireHost; // The host name where the Openfire service runs
    private int m_openfireXmlRpcPort; // The port number to use for XML-RPC Openfire requests
    private String m_sipxSupervisorHost;//The host name where SipX Supervisor runs.
    private int  m_sipxSupervisorXmlRpcPort;// The port number to use for XML-RPC SipX Supervisor alarm requests
    private String m_configAddress;//The IP where sipXconfig runs.
    private String m_binDirectory;
    private String m_logDirectory;
    private String m_backupPath;

    public String getLogLevel() {
        return SipFoundryLayout.getSipFoundryLogLevel(this.getClass()).toString();
    }

    public String getLogFile() {
        return m_logFile;
    }

    public void setLogFile(String logFile) {
        m_logFile = logFile;
    }

    public int getEventSocketPort() {
        return m_eventSocketPort;
    }

    public void setEventSocketPort(int port) {
        m_eventSocketPort = port;
    }

    public String getDataDirectory() {
        return m_dataDirectory;
    }

    public void setDataDirectory(String dir) {
        m_dataDirectory = dir;
    }

    public String getMailstoreDirectory() {
        return m_mailstoreDirectory;
    }

    public void setMailstoreDirectory(String dir) {
        m_mailstoreDirectory = dir;
    }

    public String getPromptsDirectory() {
        return m_promptsDirectory;
    }

    public void setPromptsDirectory(String dir) {
        m_promptsDirectory = dir;
    }

    public String getOrganizationPrefs() {
        return m_organizationPrefs;
    }

    public void setOrganizationPrefs(String prefs) {
        m_organizationPrefs = prefs;
    }

    public String getScriptsDirectory() {
        return m_scriptsDirectory;
    }

    public void setScriptsDirectory(String dir) {
        m_scriptsDirectory = dir;
    }

    public String getDocDirectory() {
        return m_docDirectory;
    }

    public void setDocDirectory(String dir) {
        m_docDirectory = dir;
    }

    public String getSendIMUrl() {
        return m_sendIMUrl;
    }

    public void setSendIMUrl(String url) {
        m_sendIMUrl = url;
    }

    public String getOperatorAddr() {
        return m_operatorAddr;
    }

    public void setOperatorAddr(String url) {
        m_operatorAddr = url;
    }

    public String getSipxchangeDomainName() {
        return m_sipxchangeDomainName;
    }

    public void setSipxchangeDomainName(String name) {
        m_sipxchangeDomainName = name;
    }

    public String getRealm() {
        return m_realm;
    }

    public void setRealm(String realm) {
        m_realm = realm;
    }

    public String getMwiUrl() {
        return m_mwiUrl;
    }

    public void setMwiUrl(String mwiUrl) {
        m_mwiUrl = mwiUrl;
    }

    public String getConfigUrl() {
        return m_configUrl;
    }

    public void setConfigUrl(String configUrl) {
        m_configUrl = configUrl;
    }

    public int getHttpPort() {
        return m_httpPort;
    }

    public void setHttpPort(int httpPort) {
        m_httpPort = httpPort;
    }

    public String getOpenfireHost() {
        return m_openfireHost;
    }

    public void setOpenfireHost(String host) {
    	m_openfireHost = host;
    }

    public int getOpenfireXmlRpcPort() {
        return m_openfireXmlRpcPort;
    }

    public void setOpenfireXmlRpcPort(int port) {
        m_openfireXmlRpcPort = port;
    }

    public String getSipxSupervisorHost() {
        return m_sipxSupervisorHost;
    }

    public void setSipxSupervisorHost(String host) {
        m_sipxSupervisorHost = host;
    }

    public String getConfigAddress() {
        return m_configAddress;
    }

    public void setConfigAddress(String config) {
        m_configAddress = config;
    }

    public String getBinDirectory() {
        return m_binDirectory;
    }

    public void setBinDirectory(String dir) {
        m_binDirectory = dir;
    }

    public String getLogDirectory() {
        return m_logDirectory;
    }

    public void setLogDirectory(String dir) {
        m_logDirectory = dir;
    }

    public String getBackupPath() {
        return m_backupPath;
    }

    public void setBackupPath(String path) {
        m_backupPath = path;
    }

    public int getSipxSupervisorXmlRpcPort() {
        return m_sipxSupervisorXmlRpcPort;
    }

    public void setSipxSupervisorXmlRpcPort(int port) {
        m_sipxSupervisorXmlRpcPort = port;
    }

}
