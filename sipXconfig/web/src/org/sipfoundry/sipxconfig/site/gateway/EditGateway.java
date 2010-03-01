/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.gateway;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.EventListener;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDevice;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.gateway.FxoPort;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.gateway.GatewayModel;
import org.sipfoundry.sipxconfig.gateway.SipTrunk;
import org.sipfoundry.sipxconfig.setting.AbstractSetting;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingSet;
import org.sipfoundry.sipxconfig.site.gateway.port.PortSettings;

/**
 * EditGateway
 */
public abstract class EditGateway extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "gateway/EditGateway";

    @InjectPage(value = PortSettings.PAGE)
    public abstract PortSettings getPortSettingsPage();

    @InjectObject(value = "spring:dialPlanContext")
    public abstract DialPlanContext getDialPlanContext();

    @InjectObject(value = "spring:gatewayContext")
    public abstract GatewayContext getGatewayContext();

    @InjectObject(value = "spring:sbcDeviceManager")
    public abstract SbcDeviceManager getSbcDeviceManager();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract Integer getGatewayId();

    public abstract void setGatewayId(Integer id);

    @Persist("session")
    public abstract Gateway getTransientGateway();

    public abstract void setTransientGateway(Gateway transientGateway);

    public abstract Gateway getGateway();

    public abstract void setGateway(Gateway gateway);

    @Persist
    public abstract Integer getRuleId();

    public abstract void setRuleId(Integer id);

    @Persist
    public abstract void setGatewayModel(GatewayModel model);

    public abstract GatewayModel getGatewayModel();

    @Persist
    public abstract String getCurrentSettingSetName();

    public abstract void setCurrentSettingSetName(String settingName);

    public abstract void setCurrentSettingSet(SettingSet currentSettingSet);

    public abstract SettingSet getCurrentSettingSet();

    @InitialValue(value = "literal:config")
    public abstract void setActiveTab(String tab);

    public abstract String getActiveTab();

    public abstract String getCurrentTab();

    public abstract void setActiveSetting(String setting);

    public abstract void setSelectedSbcDevice(SbcDevice selectedSbcDevice);

    public abstract SbcDevice getSelectedSbcDevice();

    /**
     * Names of the tabs that are not in navigation components
     */
    public String[] getTabNames() {
        String[] tabs = new String[] {
            "config", "gcai", "dialplan"
        };
        if (getGateway().getModel().getMaxPorts() > 0) {
            tabs = (String[]) ArrayUtils.add(tabs, 1, "ports");
        }
        return tabs;
    }

    @EventListener(events = "onchange", targets = "sbcDeviceSelect")
    public void onSbcDeviceSelect() {
        setTransientGateway(getGateway());
    }

    public void pageBeginRender(PageEvent event_) {
        if (getTransientGateway() != null) {
            setGateway(getTransientGateway());
        }
        setTransientGateway(null);

        Gateway gateway = getGateway();
        if (null != gateway) {
            return;
        }
        Integer id = getGatewayId();
        GatewayContext gatewayContext = getGatewayContext();
        if (null != id) {
            gateway = getGatewayContext().getGateway(id);
        } else {
            gateway = gatewayContext.newGateway(getGatewayModel());
        }
        setGateway(gateway);
        setSettingProperties(getCurrentSettingSetName());
        if (gateway instanceof SipTrunk) {
            SbcDevice sbcDevice = ((SipTrunk) gateway).getSbcDevice();
            if (sbcDevice != null) {
                // This is an ITSP SIP Trunk
                setSelectedSbcDevice(sbcDevice);
            } else {
                // This is Direct SIP Trunk
                Setting itsp = gateway.getSettings().getSetting("itsp-account");
                if (itsp != null) {
                    ((AbstractSetting) itsp).setHidden(true);
                }
            }
        }
    }

    public void editNonSettings(String tabId) {
        setCurrentSettingSetName(null);
        setActiveTab(tabId);
    }

    public void editSettings(Integer gatewayId, String settingPath) {
        setActiveTab("settings");
        setGatewayId(gatewayId);
        setGateway(getGatewayContext().getGateway(gatewayId));
        setSettingProperties(settingPath);
    }

    private void setSettingProperties(String settingPath) {
        SettingSet currentSettingSet = null;
        String currentSettingSetName = null;
        Setting settings = getGateway().getSettings();
        // because setting path is persistent in session, guard against
        // path not relevant to this gateway's setting set
        if (settings != null && !StringUtils.isBlank(settingPath)) {
            currentSettingSet = (SettingSet) settings.getSetting(settingPath);
            if (currentSettingSet != null) {
                currentSettingSetName = currentSettingSet.getName();
            }
        }

        setCurrentSettingSet(currentSettingSet);
        setCurrentSettingSetName(currentSettingSetName);
    }

    public void saveGateway() {
        Gateway gateway = getGateway();
        GatewayContext gatewayContext = getGatewayContext();
        // set sbc device only for SipTrunk
        if (gateway instanceof SipTrunk) {
            SbcDevice sbcDevice = getSelectedSbcDevice();
            if (sbcDevice != null) {
                gateway.setSbcDevice(sbcDevice);
            }
        }
        gatewayContext.storeGateway(gateway);

        // attach gateway to current rule
        Integer ruleId = getRuleId();
        if (null != ruleId) {
            DialPlanContext manager = getDialPlanContext();
            DialingRule rule = manager.getRule(ruleId);
            rule.addGateway(gateway);
            manager.storeRule(rule);
        }
        // refresh gateway - it cannot be new any more
        if (getGatewayId() == null) {
            setGatewayId(gateway.getId());
            setGateway(null);
        }
    }

    public IPage addPort() {
        Gateway gateway = getGateway();
        gateway.addPort(new FxoPort());
        getGatewayContext().storeGateway(gateway);
        int last = gateway.getPorts().size() - 1;
        FxoPort port = gateway.getPorts().get(last);

        return editPort(port.getId());
    }

    public IPage editPort(Integer portId) {
        PortSettings editPortPage = getPortSettingsPage();
        editPortPage.setParentSettingName(null);
        editPortPage.setPortId(portId);
        editPortPage.setRuleId(getRuleId());
        editPortPage.setReturnPage(getPage());
        return editPortPage;
    }

    public static EditGateway getEditPage(IRequestCycle cycle, Integer gatewayId, IPage returnPage, Integer ruleId) {
        EditGateway page = (EditGateway) cycle.getPage(PAGE);
        page.setGatewayModel(null);
        page.setGatewayId(gatewayId);
        page.setGateway(null);
        page.setCurrentSettingSetName(null);
        page.setRuleId(ruleId);
        page.setReturnPage(returnPage);
        page.setTransientGateway(null);
        return page;
    }

    public static EditGateway getAddPage(IRequestCycle cycle, GatewayModel model, IPage returnPage, Integer ruleId) {
        EditGateway page = (EditGateway) cycle.getPage(PAGE);
        page.setGatewayModel(model);
        page.setGatewayId(null);
        page.setGateway(null);
        page.setCurrentSettingSetName(null);
        page.setRuleId(ruleId);
        page.setReturnPage(returnPage);
        page.setTransientGateway(null);
        return page;
    }
}
