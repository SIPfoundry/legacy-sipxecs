/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site;

import org.apache.tapestry.PageRedirectException;
import org.apache.tapestry.RedirectException;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectState;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.site.user.UserRegistrations;
import org.sipfoundry.sipxconfig.site.vm.ManageVoicemail;

public abstract class Home extends SipxBasePage implements PageBeginRenderListener {
    public static final String PAGE = "Home";

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectState(value = "userSession")
    public abstract UserSession getUserSession();

    @InjectObject("spring:featureManager")
    public abstract FeatureManager getFeatureManager();

    @InjectObject(value = "spring:adminContext")
    public abstract AdminContext getAdminContext();

    public abstract void setShowForbiddenMessage(boolean forbidden);

    @InitialValue(value = "false")
    public abstract boolean getShowForbiddenMessage();

    @Override
    public void pageBeginRender(PageEvent event) {
        if (!getUserSession().isAdmin()) {
            Setting adminSettings = getAdminContext().getSettings().getSettings();
            Setting oldPortal = adminSettings.getSetting("user-portal/old-portal");
            if (oldPortal != null) {
                boolean oldPortalEnabled = (Boolean) oldPortal.getTypedValue();
                if (!oldPortalEnabled) {
                    throw new RedirectException("/unitelite");
                }
            }
            if (getFeatureManager().isFeatureEnabled(Ivr.FEATURE)) {
                throw new PageRedirectException(ManageVoicemail.PAGE);
            }
            throw new PageRedirectException(UserRegistrations.PAGE);
        }
    }

}
