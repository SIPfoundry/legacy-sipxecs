/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.gateway;

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.enums.Enum;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDevice;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.EnumUserType;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Gateway
 */
public class Gateway extends Device implements NamedObject {
    private String m_name;

    private String m_address;

    private int m_addressPort;

    private String m_outboundAddress;

    private int m_outboundPort = 5060;

    private AddressTransport m_addressTransport = AddressTransport.NONE;

    private String m_prefix;

    private String m_description;

    private GatewayModel m_model;

    private ModelSource<GatewayModel> m_modelSource;

    private GatewayCallerAliasInfo m_callerAliasInfo = new GatewayCallerAliasInfo();

    private List<FxoPort> m_ports = new ArrayList<FxoPort>();

    private SbcDevice m_sbcDevice;

    private boolean m_shared = true; // default enabled

    private boolean m_enabled = true;

    private Branch m_branch;

    private boolean m_useSipXBridge = true; // default enabled


    public Gateway() {
    }

    public Gateway(GatewayModel model) {
        setModel(model);
    }

    public Gateway(ModelSource<GatewayModel> modelSource, String modelId) {
        setGatewayModelSource(modelSource);
        setModelId(modelId);
    }

    @Override
    public void initialize() {
    }

    public void initializePort(@SuppressWarnings("unused") FxoPort port) {
    }

    public static final class AddressTransport extends Enum {
        public static final AddressTransport NONE = new AddressTransport("none");
        public static final AddressTransport UDP = new AddressTransport("udp");
        public static final AddressTransport TCP = new AddressTransport("tcp");
        public static final AddressTransport TLS = new AddressTransport("tls");

        public AddressTransport(String name) {
            super(name);
        }

        public static AddressTransport getEnum(String type) {
            return (AddressTransport) getEnum(AddressTransport.class, type);
        }
    }

    /**
     * Used for Hibernate type translation
     */
    public static class UserType extends EnumUserType {
        public UserType() {
            super(AddressTransport.class);
        }
    }

    public AddressTransport getAddressTransport() {
        return m_addressTransport;
    }

    public void setAddressTransport(AddressTransport addressTransport) {
        m_addressTransport = addressTransport;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getAddress() {
        return m_address;
    }

    public void setAddress(String address) {
        m_address = address;
    }

    public int getAddressPort() {
        return m_addressPort;
    }

    public void setAddressPort(int port) {
        m_addressPort = port;
    }

    public String getOutboundAddress() {
        return m_outboundAddress;
    }

    public void setOutboundAddress(String address) {
        m_outboundAddress = address;
    }

    public int getoutboundPort() {
        return m_outboundPort;
    }

    public void setOutboundPort(int port) {
        m_outboundPort = port;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public List<FxoPort> getPorts() {
        return m_ports;
    }

    public void setPorts(List<FxoPort> ports) {
        m_ports = ports;
    }

    public void setModel(GatewayModel model) {
        m_model = model;
        setModelId(m_model.getModelId());
        setBeanId(m_model.getBeanId());
    }

    @Override
    public GatewayModel getModel() {
        if (m_model != null) {
            return m_model;
        }
        if (getModelId() == null) {
            throw new IllegalStateException("Model ID not set");
        }
        if (m_modelSource == null) {
            throw new IllegalStateException("ModelSource not set");
        }
        m_model = m_modelSource.getModel(getModelId());
        return m_model;
    }

    public String getPrefix() {
        return m_prefix;
    }

    public void setPrefix(String prefix) {
        m_prefix = prefix;
    }

    /**
     * Used to set gateway address incorporates address and port of the gateway.
     */
    public String getGatewayAddress() {
        StringBuffer addr = new StringBuffer();
        if (m_address != null) {
            addr.append(m_address);
            if (m_addressPort > 0) {
                addr.append(":");
                addr.append(m_addressPort);
            }
        }
        return addr.toString();
    }

    /**
     * Used to obtain transport URL parameter for the gateway
     */
    public String getGatewayTransportUrlParam() {
        if (getAddressTransport().equals(Gateway.AddressTransport.NONE)) {
            return null;
        }
        return String.format("transport=%s", getAddressTransport().getName());
    }

    /**
     * Used to set header parameter route in fallback rules when generating rules for this
     * gateway.
     */
    public String getRoute() {
        return null;
    }

    public SbcDevice getSbcDevice() {
        return m_sbcDevice;
    }

    public void setSbcDevice(SbcDevice sbcDevice) {
        this.m_sbcDevice = sbcDevice;
    }

    public GatewayCallerAliasInfo getCallerAliasInfo() {
        return m_callerAliasInfo;
    }

    public void setCallerAliasInfo(GatewayCallerAliasInfo callerAliasInfo) {
        m_callerAliasInfo = callerAliasInfo;
    }

    public boolean getUseSipXBridge() {
        return m_useSipXBridge;
    }

    public void setUseSipXBridge(boolean useSipXbridge) {
        m_useSipXBridge = useSipXbridge;
    }

    @Override
    protected Setting loadSettings() {
        return null;
    }

    public Setting loadPortSettings() {
        return null;
    }

    /**
     * Prepends gateway specific call pattern to call pattern.
     */
    public String getCallPattern(String callPattern) {
        if (StringUtils.isEmpty(m_prefix)) {
            return callPattern;
        }
        return m_prefix + callPattern;
    }

    @Override
    protected Object clone() throws CloneNotSupportedException {
        Gateway clone = (Gateway) super.clone();
        clone.m_callerAliasInfo = (GatewayCallerAliasInfo) m_callerAliasInfo.clone();
        return clone;
    }

    public void setGatewayModelSource(ModelSource<GatewayModel> modelSource) {
        m_modelSource = modelSource;
    }

    public void addPort(FxoPort port) {
        List<FxoPort> ports = getPorts();
        int maxPorts = getModel().getMaxPorts();
        if (ports.size() >= maxPorts) {
            throw new MaxPortsException(maxPorts);
        }
        if (ports.add(port)) {
            port.setGateway(this);
        }
    }

    public void removePort(FxoPort port) {
        if (getPorts().remove(port)) {
            port.setGateway(null);
        }
    }

    private static class MaxPortsException extends UserException {
        public MaxPortsException(int max) {
            super("Maximum number of ports is {0}", max);
        }
    }

    public boolean isShared() {
        return m_shared;
    }

    public void setShared(boolean shared) {
        m_shared = shared;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public Branch getBranch() {
        return m_branch;
    }

    public void setBranch(Branch branch) {
        m_branch = branch;
    }
}
