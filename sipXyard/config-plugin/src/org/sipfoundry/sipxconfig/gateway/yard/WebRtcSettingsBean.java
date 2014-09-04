/**
 *
 *
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.gateway.yard;

import org.codehaus.jackson.annotate.JsonProperty;

public class WebRtcSettingsBean {
    @JsonProperty("log-level")
    private String m_logLevel;

    @JsonProperty("ip-address")
    private String m_ipAddress;

    private String m_realm;

    private String m_domain;

    @JsonProperty("proxy-address")
    private String m_proxyAddress;

    @JsonProperty("proxy-port")
    private String m_proxyPort;

    @JsonProperty("rpc-url")
    private String m_rpcUrl;

    @JsonProperty("ws-port")
    private String m_wsPort;

    @JsonProperty("tcp-udp-port")
    private String m_tcpUdpPort;

    @JsonProperty("bridge-tcp-udp-port")
    private String m_bridgeTcpUdpPort;

    @JsonProperty("enable-library-logging")
    private String m_enableLibraryLogging;

    @JsonProperty("user-cache")
    private String m_userCache;

    @JsonProperty("db-path")
    private String m_dbPath;

    @JsonProperty("bridge-esl-port")
    private String m_bridgeEslPort;

    @JsonProperty("switch-esl-port")
    private String m_switchEslPort;

    public String getWsPort() {
        return m_wsPort;
    }

    public void setWsPort(String wsPort) {
        m_wsPort = wsPort;
    }

    public String getLogLevel() {
        return m_logLevel;
    }

    public void setLogLevel(String logLevel) {
        m_logLevel = logLevel;
    }

    public String getIpAddress() {
        return m_ipAddress;
    }

    public void setIpAddress(String ipAddress) {
        m_ipAddress = ipAddress;
    }

    public String getRealm() {
        return m_realm;
    }

    public void setRealm(String realm) {
        m_realm = realm;
    }

    public String getDomain() {
        return m_domain;
    }

    public void setDomain(String domain) {
        m_domain = domain;
    }

    public String getProxyAddress() {
        return m_proxyAddress;
    }

    public void setProxyAddress(String proxyAddress) {
        m_proxyAddress = proxyAddress;
    }

    public String getProxyPort() {
        return m_proxyPort;
    }

    public void setProxyPort(String proxyPort) {
        m_proxyPort = proxyPort;
    }

    public String getRpcUrl() {
        return m_rpcUrl;
    }

    public void setRpcUrl(String rpcUrl) {
        m_rpcUrl = rpcUrl;
    }

    public String getTcpUdpPort() {
        return m_tcpUdpPort;
    }

    public void setTcpUdpPort(String tcpUdpPort) {
        m_tcpUdpPort = tcpUdpPort;
    }

    public String getBridgeTcpUdpPort() {
        return m_bridgeTcpUdpPort;
    }

    public void setBridgeTcpUdpPort(String bridgeTcpUdpPort) {
        m_bridgeTcpUdpPort = bridgeTcpUdpPort;
    }

    public String getEnableLibraryLogging() {
        return m_enableLibraryLogging;
    }

    public void setEnableLibraryLogging(String enableLibraryLogging) {
        m_enableLibraryLogging = enableLibraryLogging;
    }

    public String getUserCache() {
        return m_userCache;
    }

    public void setUserCache(String userCache) {
        m_userCache = userCache;
    }

    public String getDbPath() {
        return m_dbPath;
    }

    public void setDbPath(String dbPath) {
        m_dbPath = dbPath;
    }

    public String getBridgeEslPort() {
        return m_bridgeEslPort;
    }

    public void setBridgeEslPort(String bridgeEslPort) {
        m_bridgeEslPort = bridgeEslPort;
    }

    public String getSwitchEslPort() {
        return m_switchEslPort;
    }

    public void setSwitchEslPort(String switchEslPort) {
        m_switchEslPort = switchEslPort;
    }
}
