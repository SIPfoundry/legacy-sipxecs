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

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.gateway.GatewayModel;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingSet;

/**
 * EditGateway
 */
public abstract class EditGateway extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "EditGateway";

    @InjectObject(value = "spring:dialPlanContext")
    public abstract DialPlanContext getDialPlanContext();

    @InjectObject(value = "spring:gatewayContext")
    public abstract GatewayContext getGatewayContext();
    
    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract Integer getGatewayId();

    public abstract void setGatewayId(Integer id);

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

    public void pageBeginRender(PageEvent event_) {
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
        // because setting path is persistant in session, guard against
        // path not rellevant to this gateways setting set
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

    public static EditGateway getEditPage(IRequestCycle cycle, Integer gatewayId,
            IPage returnPage, Integer ruleId) {
        EditGateway page = (EditGateway) cycle.getPage(PAGE);
        page.setGatewayModel(null);
        page.setGatewayId(gatewayId);
        page.setGateway(null);
        page.setRuleId(ruleId);
        page.setReturnPage(returnPage);
        return page;
    }

    public static EditGateway getAddPage(IRequestCycle cycle, GatewayModel model, IPage returnPage,
            Integer ruleId) {
        EditGateway page = (EditGateway) cycle.getPage(PAGE);
        page.setGatewayModel(model);
        page.setGatewayId(null);
        page.setGateway(null);
        page.setRuleId(ruleId);
        page.setReturnPage(returnPage);
        return page;
    }
}
