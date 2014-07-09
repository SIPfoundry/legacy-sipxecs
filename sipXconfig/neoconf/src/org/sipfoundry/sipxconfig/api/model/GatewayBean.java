/**
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
package org.sipfoundry.sipxconfig.api.model;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlType;

import org.codehaus.jackson.annotate.JsonPropertyOrder;
import org.sipfoundry.sipxconfig.device.DeviceDescriptor;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayCallerAliasInfo;

@XmlRootElement(name = "Gateway")
@XmlType(propOrder = {
        "id", "name", "serialNo", "deviceVersion", "description", "model", "enabled", "address", "addressPort",
        "outboundAddress", "outboundPort", "addressTransport", "prefix", "shared",
        "useInternalBridge", "branch", "callerAliasInfo"
        })
@JsonPropertyOrder({
        "id", "name", "serialNo", "deviceVersion", "description", "model", "enabled", "address", "addressPort",
        "outboundAddress", "outboundPort", "addressTransport", "prefix", "shared",
        "useInternalBridge", "branch", "callerAliasInfo"
       })
public class GatewayBean {
    private int m_id;
    private String m_name;
    private String m_description;
    private boolean m_enabled;
    private String m_serialNo;
    private String m_address;
    private int m_addressPort;
    private String m_outboundAddress;
    private int m_outboundPort;
    private String m_addressTransport;
    private String m_prefix;
    private BranchBean m_branch;
    private boolean m_shared;
    private boolean m_useInternalBridge;
    private ModelBean m_model;
    private GatewayCallerAliasInfo m_callerAliasInfo;
    private String m_version;

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        m_id = id;
    }

    public String getSerialNo() {
        return m_serialNo;
    }

    public void setSerialNo(String serial) {
        m_serialNo = serial;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public String getAddress() {
        return m_address;
    }

    public void setAddress(String address) {
        m_address = address;
    }

    @XmlElement(name = "Branch")
    public BranchBean getBranch() {
        return m_branch;
    }

    public void setBranch(BranchBean branch) {
        m_branch = branch;
    }

    @XmlElement(name = "Model")
    public ModelBean getModel() {
        return m_model;
    }

    public void setModel(ModelBean model) {
        m_model = model;
    }

    public int getAddressPort() {
        return m_addressPort;
    }

    public void setAddressPort(int addressPort) {
        m_addressPort = addressPort;
    }

    public String getOutboundAddress() {
        return m_outboundAddress;
    }

    public void setOutboundAddress(String outboundAddress) {
        m_outboundAddress = outboundAddress;
    }

    public int getOutboundPort() {
        return m_outboundPort;
    }

    public void setOutboundPort(int outboundPort) {
        m_outboundPort = outboundPort;
    }

    public String getAddressTransport() {
        return m_addressTransport;
    }

    public void setAddressTransport(String addressTransport) {
        m_addressTransport = addressTransport;
    }

    public String getPrefix() {
        return m_prefix;
    }

    public void setPrefix(String prefix) {
        m_prefix = prefix;
    }

    public boolean isShared() {
        return m_shared;
    }

    public void setShared(boolean shared) {
        m_shared = shared;
    }

    public boolean isUseInternalBridge() {
        return m_useInternalBridge;
    }

    public void setUseInternalBridge(boolean useInternalBridge) {
        m_useInternalBridge = useInternalBridge;
    }

    @XmlElement(name = "CallerAliasInfo")
    public GatewayCallerAliasInfo getCallerAliasInfo() {
        return m_callerAliasInfo;
    }

    public void setCallerAliasInfo(GatewayCallerAliasInfo callerAliasInfo) {
        m_callerAliasInfo = callerAliasInfo;
    }

    public String getDeviceVersion() {
        return m_version;
    }

    public void setDeviceVersion(String version) {
        m_version = version;
    }

    public static GatewayBean convertGateway(Gateway gateway) {
        DeviceDescriptor gatewayModel = gateway.getModel();
        GatewayBean bean = new GatewayBean();
        bean.setId(gateway.getId());
        bean.setName(gateway.getName());
        bean.setDescription(gateway.getDescription());
        bean.setEnabled(gateway.isEnabled());
        bean.setAddress(gateway.getAddress());
        bean.setAddressPort(gateway.getAddressPort());
        bean.setOutboundAddress(gateway.getOutboundAddress());
        bean.setOutboundPort(gateway.getoutboundPort());
        bean.setAddressTransport(gateway.getAddressTransport().getName());
        bean.setBranch(BranchBean.convertBranch(gateway.getBranch()));
        bean.setPrefix(gateway.getPrefix());
        bean.setShared(gateway.isShared());
        bean.setUseInternalBridge(gateway.getUseSipXBridge());
        bean.setModel(ModelBean.convertModel(gatewayModel));
        bean.setCallerAliasInfo(gateway.getCallerAliasInfo());
        if (gateway.getDeviceVersion() != null) {
            bean.setDeviceVersion(gateway.getDeviceVersion().getName());
        }
        return bean;
    }
}
