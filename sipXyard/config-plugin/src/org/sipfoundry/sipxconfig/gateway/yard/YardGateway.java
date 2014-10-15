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

import java.io.IOException;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.apache.commons.httpclient.HttpClient;
import org.apache.commons.httpclient.HttpConnectionManager;
import org.apache.commons.httpclient.HttpMethodBase;
import org.apache.commons.httpclient.HttpStatus;
import org.apache.commons.httpclient.SimpleHttpConnectionManager;
import org.apache.commons.httpclient.methods.GetMethod;
import org.apache.commons.httpclient.methods.PostMethod;
import org.apache.commons.httpclient.params.HttpClientParams;
import org.apache.commons.httpclient.params.HttpConnectionManagerParams;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.codehaus.jackson.map.DeserializationConfig;
import org.codehaus.jackson.map.ObjectMapper;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.device.ProfileLocation;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.WebRtcGateway;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.sipfoundry.sipxconfig.sipxedgerpc.Sipxedgerpc;
import org.springframework.beans.factory.annotation.Required;

public class YardGateway extends Gateway implements WebRtcGateway {
    private static final Log LOG = LogFactory.getLog(YardGateway.class);
    private static final String WEBRTC_LOG_LEVEL = "webrtc/log-level";
    private static final String WEBRTC_WS_PORT = "webrtc/ws-port";
    private static final String WEBRTC_TCP_UDP_PORT = "webrtc/tcp-udp-port";
    private static final String WEBRTC_BRIDGE_TCP_UDP_PORT = "webrtc/bridge-tcp-udp-port";
    private static final String WEBRTC_ENABLE_LIBRARY_LOGGING = "webrtc/enable-library-logging";
    private static final String WEBRTC_USER_CACHE = "webrtc/user-cache";
    private static final String WEBRTC_DB_PATH = "webrtc/db-path";
    private static final String WEBRTC_BRIDGE_ESL_PORT = "webrtc/bridge-esl-port";
    private static final String WEBRTC_SWITCH_ESL_PORT = "webrtc/switch-esl-port";

    private static final int DEFAULT_ADDRESS_PORT = 8020;
    private ExecutorService m_service = Executors.newSingleThreadExecutor();
    private WebRtcBean m_bean;
    private AddressManager m_addressManager;
    private DomainManager m_domainManager;
    private FeatureManager m_featureManager;
    private LocationsManager m_locationManager;

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("yard-gateway.xml", getModel().getModelDir());
    }

    @Override
    public void initialize() {
        setAddressPort(DEFAULT_ADDRESS_PORT);
        String response = executeGet();
        if (!StringUtils.isEmpty(response)) {
            m_bean = convertJson(response);
        }
        addDefaultBeanSettingHandler(new YardDefaults());
        //automatically start edge RPC service on master node if not already started
        //rpc service can run on any node
        if (!m_featureManager.isFeatureEnabled(Sipxedgerpc.FEATURE)) {
            m_featureManager.enableLocationFeature(Sipxedgerpc.FEATURE, m_locationManager.getPrimaryLocation(), true);
        }
    }

    @Override
    public void generateProfiles(ProfileLocation location) {
        executePost("ip-address", getAddress());
        executePost("realm", m_domainManager.getDomain().getSipRealm());
        executePost("domain", m_domainManager.getDomain().getName());
        executePost("ws-port", getWsPort());
        executePost("proxy-address", m_domainManager.getDomain().getName());
        executePost("rpc-url", getEdgeRpcUris());
        executePost("user-cache", getSettings().getSetting(WEBRTC_USER_CACHE).getValue());
        executePost("log-level", getSettings().getSetting(WEBRTC_LOG_LEVEL).getValue());
    }

    @Override
    protected void beforeProfileGeneration() {
        return;
    }

    @Override
    protected void copyFiles(ProfileLocation location) {
    }

    @Override
    public void generateFiles(ProfileLocation location) {
    }

    public String executePost(String property, String value) {
        if (value == null) {
            return null;
        }
        HttpClient client = getHttpClient();
        PostMethod postMethod = new PostMethod(getYardURI(property));
        postMethod.addParameter("value", value);
        int statusCode = HttpStatus.SC_OK;
        String response = StringUtils.EMPTY;

        statusCode = execute(client, postMethod);
        if (statusCode == HttpStatus.SC_OK) {
            try {
                response = IOUtils.toString(postMethod.getResponseBodyAsStream(), "UTF-8");
            } catch (IOException e) {
                LOG.error("Cannot execute POST ", e);
            }
        }
        return response;
    }

    public String executeGet() {
        HttpClient client = getHttpClient();
        HttpClientParams params = client.getParams();
        params.setSoTimeout(10000);
        params.setConnectionManagerTimeout(10000);
        GetMethod getMethod = new GetMethod(getYardURI());
        int statusCode = HttpStatus.SC_OK;
        String response = StringUtils.EMPTY;

        statusCode = execute(client, getMethod);
        if (statusCode == HttpStatus.SC_OK) {
            try {
                response = IOUtils.toString(getMethod.getResponseBodyAsStream(), "UTF-8");
            } catch (Exception e) {
                LOG.error("Cannot execute GET ", e);
            }
        }

        return response;
    }

    private String getYardURI(String property) {
        return String.format("http://%s:%d/root/system-config/webrtc/%s", getAddress(), getAddressPort(), property);
    }

    private String getYardURI() {
        return String.format("http://%s:%d/root/system-config/webrtc/", getAddress(), getAddressPort());
    }

    private HttpClient getHttpClient() {
        HttpConnectionManagerParams cmparams = new HttpConnectionManagerParams();
        cmparams.setSoTimeout(10000);
        cmparams.setTcpNoDelay(true);
        HttpConnectionManager manager = new SimpleHttpConnectionManager();
        manager.setParams(cmparams);
        HttpClientParams params = new HttpClientParams();
        params.setSoTimeout(10000);
        HttpClient client = new HttpClient(params, manager);
        return client;
    }

    private int execute(final HttpClient client, final HttpMethodBase method) {
        Future<Integer> future = null;
        future = m_service.submit(new Callable<Integer>() {
            @Override
            public Integer call() {
                try {
                    return client.executeMethod(method);
                } catch (Exception e) {
                    return HttpStatus.SC_BAD_REQUEST;
                }
            }
        });
        try {
            return future.get(10, TimeUnit.SECONDS);
        }
        catch (InterruptedException e) {
            return HttpStatus.SC_BAD_REQUEST;
        }
        catch (ExecutionException e) {
            return HttpStatus.SC_BAD_REQUEST;
        }
        catch (TimeoutException e) {
            return HttpStatus.SC_REQUEST_TIMEOUT;
        }
    }

    private WebRtcBean convertJson(String json) {
        ObjectMapper mapper = new ObjectMapper();
        mapper.configure(DeserializationConfig.Feature.FAIL_ON_UNKNOWN_PROPERTIES, false);
        try {
            return mapper.readValue(json, WebRtcBean.class);
        } catch (Exception e) {
            LOG.error("Cannot convert JSON ", e);
            return null;
        }
    }

    private String getEdgeRpcUris() {
        List<Address> addresses = m_addressManager.getAddresses(Sipxedgerpc.HTTP_ADDRESS);
        String[] addressesArray = new String[addresses.size()];
        int i=0;
        for (Address address : addresses) {
            addressesArray[i++] = address.toString();
        }
        return StringUtils.join(addressesArray, ",");
    }


    @Required
    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    @Required
    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    @Required
    public void setLocationManager(LocationsManager locationManager) {
        m_locationManager = locationManager;
    }

    public class YardDefaults {
        @SettingEntry(path = WEBRTC_LOG_LEVEL)
        public String getLogLevel() {
            return m_bean != null ? m_bean.getWebRtc().getLogLevel() : "N/A";
        }

        @SettingEntry(path = WEBRTC_WS_PORT)
        public String getWsPort() {
            return m_bean != null ? m_bean.getWebRtc().getWsPort() : "N/A";
        }

        @SettingEntry(path = WEBRTC_TCP_UDP_PORT)
        public String getTcpUdpPort() {
            return m_bean != null ? m_bean.getWebRtc().getTcpUdpPort() : "N/A";
        }

        @SettingEntry(path = WEBRTC_BRIDGE_TCP_UDP_PORT)
        public String getBridgeTcpUdpPort() {
            return m_bean != null ? m_bean.getWebRtc().getBridgeTcpUdpPort() : "N/A";
        }

        @SettingEntry(path = WEBRTC_ENABLE_LIBRARY_LOGGING)
        public boolean getEnableLibraryLogging() {
            return m_bean != null ? new Boolean(m_bean.getWebRtc().getEnableLibraryLogging()) : false;
        }

        @SettingEntry(path = WEBRTC_USER_CACHE)
        public String getUserCache() {
            return m_bean != null ? m_bean.getWebRtc().getUserCache() : "N/A";
        }

        @SettingEntry(path = WEBRTC_DB_PATH)
        public String getDbPath() {
            return m_bean != null ? m_bean.getWebRtc().getDbPath() : "N/A";
        }

        @SettingEntry(path = WEBRTC_BRIDGE_ESL_PORT)
        public String getBridgeEslPort() {
            return m_bean != null ? m_bean.getWebRtc().getBridgeEslPort() : "N/A";
        }

        @SettingEntry(path = WEBRTC_SWITCH_ESL_PORT)
        public String getSwitchEslPort() {
            return m_bean != null ? m_bean.getWebRtc().getSwitchEslPort() : "N/A";
        }
    }

    @Override
    public String getWsPort() {
        return getYardValue(WEBRTC_WS_PORT);
    }

    private String getYardValue(String key) {
        return getSettings().getSetting(key).getValue();
    }
}
