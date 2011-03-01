/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.Arrays;
import java.util.List;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxService;

public abstract class LocalizationPage extends SipxBasePage implements PageBeginRenderListener {
    public static final String PAGE = "admin/LocalizationPage";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject("spring:localizationContext")
    public abstract LocalizationContext getLocalizationContext();

    @InjectObject("spring:dialPlanContext")
    public abstract DialPlanContext getDialPlanContext();

    @InjectObject("spring:sipxProcessContext")
    public abstract SipxProcessContext getProcessContext();

    @InjectObject("spring:sipxRegistrarService")
    public abstract SipxRegistrarService getSipxRegistrarService();

    @InjectObject("spring:sipxProxyService")
    public abstract SipxProxyService getSipxProxyService();

    public abstract IPropertySelectionModel getRegionList();

    public abstract void setRegionList(IPropertySelectionModel regionList);

    public abstract String getRegion();

    public abstract String getLanguage();

    public abstract void setLanguage(String language);

    public abstract void setRegion(String region);

    public void pageBeginRender(PageEvent event_) {
        if (getRegionList() == null) {
            initRegions();
        }

        if (getRegion() == null) {
            String defaultRegion = getLocalizationContext().getLocalization().getRegion();
            setRegion(defaultRegion);
        }

        if (getLanguage() == null) {
            String defaultLanguage = getLocalizationContext().getCurrentLanguage();
            setLanguage(defaultLanguage);
        }
    }

    private void initRegions() {
        String[] regions = getDialPlanContext().getDialPlanBeans();
        IPropertySelectionModel model = new ModelWithDefaults(getMessages(), regions);
        setRegionList(model);
    }

    public IPage changeRegion(IRequestCycle cycle) {
        String region = getRegion();
        if (ModelWithDefaults.DEFAULT.equals(region)) {
            return getPage();
        }

        ConfirmUpdateRegion updatePage = (ConfirmUpdateRegion) cycle.getPage(ConfirmUpdateRegion.PAGE);
        updatePage.setRegion(getRegion());
        updatePage.setReturnPage(PAGE);
        return updatePage;
    }

    public void changeLanguage() {
        String language = getLanguage();
        int exitCode = getLocalizationContext().updateLanguage(language);

        if (exitCode > 0) {
            List< ? extends SipxService> processList = Arrays.asList(getSipxProxyService(),
                    getSipxRegistrarService());
            // mark services for restart - a reminder will be shown to the user
            getProcessContext().markServicesForRestart(processList);
            recordSuccess("message.label.languageChanged");
        } else if (exitCode < 0) {
            recordFailure("message.label.languageFailed");
        }
    }

    private void recordSuccess(String msgKey) {
        String msg = getMessages().getMessage(msgKey);
        TapestryUtils.recordSuccess(this, msg);
    }

    private void recordFailure(String msgKey) {
        String msg = getMessages().getMessage(msgKey);
        ValidatorException validatorException = new ValidatorException(msg);
        getValidator().record(validatorException);
    }
}
