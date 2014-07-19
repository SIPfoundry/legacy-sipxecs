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
package org.sipfoundry.sipxconfig.api.impl;

import java.util.Collection;
import java.util.List;

import javax.servlet.http.HttpServletRequest;
import javax.ws.rs.core.Response;
import javax.ws.rs.core.Response.Status;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.api.GatewayApi;
import org.sipfoundry.sipxconfig.api.model.BranchBean;
import org.sipfoundry.sipxconfig.api.model.DialingRuleList;
import org.sipfoundry.sipxconfig.api.model.FxoPortBean.FxoPortList;
import org.sipfoundry.sipxconfig.api.model.GatewayBean;
import org.sipfoundry.sipxconfig.api.model.GatewayList;
import org.sipfoundry.sipxconfig.api.model.ModelBean;
import org.sipfoundry.sipxconfig.api.model.ModelBean.ModelList;
import org.sipfoundry.sipxconfig.api.model.SettingsList;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.branch.BranchManager;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.gateway.FxoPort;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.Gateway.AddressTransport;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.gateway.GatewayModel;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.beans.factory.annotation.Required;

public class GatewayApiImpl implements GatewayApi {

    private static final Log LOG = LogFactory.getLog(GatewayApiImpl.class);

    private GatewayContext m_gatewayContext;
    private ModelSource<GatewayModel> m_modelSource;
    private BranchManager m_branchManager;
    private DialPlanContext m_dialPlanContext;

    @Override
    public Response getGateways() {
        List<Gateway> gateways = m_gatewayContext.getGateways();
        if (gateways != null) {
            return Response.ok().entity(GatewayList.convertGatewayList(gateways)).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getGateway(String gatewayId) {
        Gateway gateway = getGatewayByIdOrSerial(gatewayId);
        if (gateway != null) {
            GatewayBean gatewayBean = GatewayBean.convertGateway(gateway);
            return Response.ok().entity(gatewayBean).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response newGateway(GatewayBean gatewayBean) {
        GatewayModel model = getGatewayModel(gatewayBean);
        if (model == null) {
            return Response.status(Status.NOT_FOUND).entity("No such phone model defined").build();
        }
        Gateway gateway = m_gatewayContext.newGateway(model);
        convertToGateway(gateway, gatewayBean);
        m_gatewayContext.saveGateway(gateway);
        return Response.ok().entity(gateway.getId()).build();
    }

    @Override
    public Response deleteGateway(String gatewayId) {
        Gateway gateway = getGatewayByIdOrSerial(gatewayId);
        if (gateway != null) {
            m_gatewayContext.deleteGateway(gateway.getId());
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response updateGateway(String gatewayId, GatewayBean gatewayBean) {
        Gateway gateway = getGatewayByIdOrSerial(gatewayId);
        if (gateway != null) {
            convertToGateway(gateway, gatewayBean);
            m_gatewayContext.saveGateway(gateway);
            return Response.ok().entity(gateway.getId()).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getGatewayModels() {
        Collection<GatewayModel> models = m_modelSource.getModels();
        if (models != null) {
            return Response.ok().entity(ModelList.convertModelList(models)).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getGatewayModelSettings(String gatewayId, String modelName, HttpServletRequest request) {

        GatewayModel gatewayModel = m_modelSource.getModel(modelName);

        if (gatewayModel != null) {
            Setting settings = m_gatewayContext.newGateway(gatewayModel).getSettings();
            return Response.ok().entity(SettingsList.convertSettingsList(settings, request.getLocale())).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getGatewaySetting(String gatewayId, String modelName, String path, HttpServletRequest request) {
        Gateway gateway = getGatewayByIdOrSerial(gatewayId);
        if (gateway != null) {
            GatewayModel gatewayModel = m_modelSource.getModel(modelName);
            Gateway phone = m_gatewayContext.newGateway(gatewayModel);
            phone.setSettingValue(path, gateway.getSettingValue(path));
            return ResponseUtils.buildSettingResponse(phone, path, request.getLocale());
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response setGatewaySetting(String gatewayId, String modelName, String path, String value) {
        Gateway gateway = getGatewayByIdOrSerial(gatewayId);
        if (gateway != null) {
            gateway.setSettingValue(path, value);
            m_gatewayContext.saveGateway(gateway);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response deleteGatewaySetting(String gatewayId, String modelName, String path) {
        Gateway gateway = getGatewayByIdOrSerial(gatewayId);
        if (gateway != null) {
            Setting setting = gateway.getSettings().getSetting(path);
            setting.setValue(setting.getDefaultValue());
            m_gatewayContext.saveGateway(gateway);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getPorts(String gatewayId) {
        Gateway gateway = getGatewayByIdOrSerial(gatewayId);
        List<FxoPort> ports = gateway.getPorts();
        if (ports != null) {
            return Response.ok().entity(FxoPortList.convertPortsList(ports)).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getGatewayPortSettings(String gatewayId, Integer portId, HttpServletRequest request) {
        Gateway gateway = getGatewayByIdOrSerial(gatewayId);
        if (gateway != null) {
            FxoPort fxoPort = getFxoPort(gateway.getPorts(), portId);
            if (fxoPort != null) {
                return Response.ok().entity(
                    SettingsList.convertSettingsList(fxoPort.getSettings(), request.getLocale())).build();
            }
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getGatewayPortSetting(String gatewayId, Integer portId, String path, HttpServletRequest request) {
        Gateway gateway = getGatewayByIdOrSerial(gatewayId);
        if (gateway != null) {
            FxoPort fxoPort = getFxoPort(gateway.getPorts(), portId);
            if (fxoPort != null) {
                return ResponseUtils.buildSettingResponse(fxoPort, path, request.getLocale());
            }
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response setGatewayPortSetting(String gatewayId, Integer portId, String path, String value) {
        Gateway gateway = getGatewayByIdOrSerial(gatewayId);
        if (gateway != null) {
            FxoPort fxoPort = getFxoPort(gateway.getPorts(), portId);
            if (fxoPort != null) {
                fxoPort.setSettingValue(path, value);
                m_gatewayContext.saveGateway(gateway);
                return Response.ok().build();
            }
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response deleteGatewayPortSetting(String gatewayId, Integer portId, String path) {
        Gateway gateway = getGatewayByIdOrSerial(gatewayId);
        if (gateway != null) {
            FxoPort fxoPort = getFxoPort(gateway.getPorts(), portId);
            if (fxoPort != null) {
                fxoPort.setSettingValue(path, fxoPort.getSettingDefaultValue(path));
                m_gatewayContext.saveGateway(gateway);
                return Response.ok().build();
            }
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getAvailableRules(String gatewayId) {
        Gateway gateway = getGatewayByIdOrSerial(gatewayId);
        if (gateway != null) {
            List<DialingRule> rules = m_dialPlanContext.getAvailableRules(gateway.getId());
            if (rules != null) {
                return Response.ok().entity(DialingRuleList.convertDialingRuleList(rules)).build();
            }

        }
        return Response.status(Status.NOT_FOUND).build();
    }

    private Gateway getGatewayByIdOrSerial(String id) {
        Gateway gateway = null;
        try {
            int gatewayId = Integer.parseInt(id);
            gateway = m_gatewayContext.getGateway(gatewayId);
        } catch (NumberFormatException e) {
            // no id then it must be serial
            Integer gId = m_gatewayContext.getGatewayIdBySerialNumber(id);
            gateway = m_gatewayContext.getGateway(gId);
        }
        return gateway;
    }

    private GatewayModel getGatewayModel(GatewayBean gatewayBean) {
        ModelBean modelBean = gatewayBean.getModel();
        String modelId = null;
        GatewayModel model = null;
        if (modelBean != null) {
            modelId = modelBean.getModelId();
            model = m_modelSource.getModel(modelId);
        }
        return model;
    }

    private Branch getBranch(GatewayBean gatewayBean) {
        BranchBean branchBean = gatewayBean.getBranch();
        String name = branchBean.getName();
        Branch branch = null;
        if (branchBean != null) {
            try {
                branch =  m_branchManager.getBranch(name);
                BranchBean.convertToBranch(gatewayBean.getBranch(), branch);
            } catch (Exception ex) {
                LOG.error("No existing branch with name " + name);
            }
        }
        return branch;
    }

    private void convertToGateway(Gateway gateway, GatewayBean gatewayBean) {
        gateway.setName(gatewayBean.getName());
        gateway.setDescription(gatewayBean.getDescription());
        gateway.setEnabled(gatewayBean.isEnabled());
        gateway.setAddress(gatewayBean.getAddress());
        gateway.setAddressPort(gatewayBean.getAddressPort());
        gateway.setOutboundAddress(gatewayBean.getOutboundAddress());
        gateway.setOutboundPort(gatewayBean.getOutboundPort());
        gateway.setAddressTransport(AddressTransport.getEnum(gatewayBean.getAddressTransport()));
        gateway.setBranch(getBranch(gatewayBean));
        gateway.setPrefix(gatewayBean.getPrefix());
        gateway.setShared(gatewayBean.isShared());
        gateway.setUseSipXBridge(gatewayBean.isUseInternalBridge());
        gateway.setModel(getGatewayModel(gatewayBean));
        gateway.setCallerAliasInfo(gatewayBean.getCallerAliasInfo());
        if (gatewayBean.getDeviceVersion() != null) {
            gateway.setDeviceVersion(new DeviceVersion(gatewayBean.getModel().getVendor(),
                gatewayBean.getDeviceVersion()));
        }
    }

    private FxoPort getFxoPort(List<FxoPort> ports, Integer id) {
        for (FxoPort port : ports) {
            if (port.getId().equals(id)) {
                return port;
            }
        }
        return null;
    }

    public void setGatewayContext(GatewayContext context) {
        m_gatewayContext = context;
    }

    public void setGatewayModelSource(ModelSource<GatewayModel> modelSource) {
        m_modelSource = modelSource;
    }

    @Required
    public void setBranchManager(BranchManager branchManager) {
        m_branchManager = branchManager;
    }

    public DialPlanContext getDialPlanContext() {
        return m_dialPlanContext;
    }

    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }
}
