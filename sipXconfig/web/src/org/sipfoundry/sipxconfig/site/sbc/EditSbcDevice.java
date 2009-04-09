/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.site.sbc;

import org.apache.commons.lang.StringUtils;
import org.apache.hivemind.util.PropertyUtils;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanActivationManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDescriptor;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDevice;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingSet;

public abstract class EditSbcDevice extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "sbc/EditSbcDevice";

    private static final String[] TAB_NAMES = {
        "config"
    };

    @InjectObject(value = "spring:sbcDeviceManager")
    public abstract SbcDeviceManager getSbcDeviceManager();

    @InjectObject("spring:dialPlanActivationManager")
    public abstract DialPlanActivationManager getDialPlanActivationManager();

    @InjectObject(value = "spring:sbcProfileManager")
    public abstract ProfileManager getProfileManager();

    @Persist
    public abstract String getReturnPageName();

    public abstract void setReturnPageName(String returnPageName);

    @Persist
    public abstract String getAddProperty();

    public abstract void setAddProperty(String addPropertyName);

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract Integer getSbcDeviceId();

    public abstract void setSbcDeviceId(Integer id);

    public abstract SbcDevice getSbcDevice();

    public abstract void setSbcDevice(SbcDevice sbcDevice);

    @Persist
    public abstract void setSbcDescriptor(SbcDescriptor model);

    public abstract SbcDescriptor getSbcDescriptor();

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

    /**
     * Names of the tabs that are not in navigation components
     */
    public String[] getTabNames() {
        return TAB_NAMES;
    }

    public void pageBeginRender(PageEvent event_) {
        SbcDevice sbc = getSbcDevice();
        if (null != sbc) {
            return;
        }
        Integer id = getSbcDeviceId();
        SbcDeviceManager sdm = getSbcDeviceManager();
        if (null != id) {
            sbc = sdm.getSbcDevice(id);
        } else {
            SbcDescriptor sbcDescriptor = getSbcDescriptor();
            sbc = sdm.newSbcDevice(sbcDescriptor);
        }
        setSbcDevice(sbc);
        setSettingProperties(getCurrentSettingSetName());
    }

    public void editNonSettings(String tabId) {
        setCurrentSettingSetName(null);
        setActiveTab(tabId);
    }

    public void editSettings(Integer sbcId, String settingPath) {
        setActiveTab("settings");
        setSbcDeviceId(sbcId);
        setSbcDevice(getSbcDeviceManager().getSbcDevice(sbcId));
        setSettingProperties(settingPath);
    }

    private void setSettingProperties(String settingPath) {
        SettingSet currentSettingSet = null;
        String currentSettingSetName = null;
        Setting settings = getSbcDevice().getSettings();
        // because setting path is persistant in session, guard against
        // path not rellevant to this SbcDevices setting set
        if (settings != null && !StringUtils.isBlank(settingPath)) {
            currentSettingSet = (SettingSet) settings.getSetting(settingPath);
            if (currentSettingSet != null) {
                currentSettingSetName = currentSettingSet.getName();
            }
        }
        setCurrentSettingSet(currentSettingSet);
        setCurrentSettingSetName(currentSettingSetName);
    }

    public void save() {
        SbcDevice sbcDevice = getSbcDevice();
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        if (getReturnPageName() != null && getAddProperty() != null) {
            IPage returnPage = getRequestCycle().getPage(getReturnPageName());
            if (PropertyUtils.isWritable(returnPage, getAddProperty())) {
                PropertyUtils.write(returnPage, getAddProperty(), sbcDevice);
            }
        }

        SbcDeviceManager sbcDeviceContext = getSbcDeviceManager();
        sbcDeviceContext.storeSbcDevice(sbcDevice);

        // refresh SbcDevice - it cannot be new any more
        if (getSbcDeviceId() == null) {
            setSbcDeviceId(sbcDevice.getId());
            setSbcDevice(null);
        }
    }

    public static EditSbcDevice getEditPage(IRequestCycle cycle, Integer sbcId, IPage returnPage) {
        EditSbcDevice page = (EditSbcDevice) cycle.getPage(PAGE);
        page.setSbcDescriptor(null);
        page.setSbcDeviceId(sbcId);
        page.setSbcDevice(null);
        page.setCurrentSettingSetName(null);
        page.setReturnPage(returnPage);
        return page;
    }

    public static EditSbcDevice getAddPage(IRequestCycle cycle, SbcDescriptor model, IPage returnPage) {
        EditSbcDevice page = (EditSbcDevice) cycle.getPage(PAGE);
        page.setSbcDescriptor(model);
        page.setSbcDeviceId(null);
        page.setSbcDevice(null);
        page.setCurrentSettingSetName(null);
        page.setReturnPage(returnPage);
        return page;
    }

    public static EditSbcDevice getAddPage(IRequestCycle cycle, SbcDescriptor model, IPage returnPage,
            String addProperty) {
        EditSbcDevice page = getAddPage(cycle, model, returnPage);
        page.setReturnPageName(returnPage.getPageName());
        page.setAddProperty(addProperty);
        return page;
    }

    @Override
    public String getBreadCrumbTitle() {
        return "&crumb.editSbcBridge";
    }
}
