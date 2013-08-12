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
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectState;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.site.user.UserRegistrations;
import org.sipfoundry.sipxconfig.site.vm.ManageVoicemail;

public abstract class Home extends SipxBasePage implements PageBeginRenderListener {
    public static final String PAGE = "Home";

    @InjectState(value = "userSession")
    public abstract UserSession getUserSession();

    @InjectObject("spring:featureManager")
    public abstract FeatureManager getFeatureManager();

    public abstract void setShowForbiddenMessage(boolean forbidden);

    @InitialValue(value = "false")
    public abstract boolean getShowForbiddenMessage();

    public void pageBeginRender(PageEvent event) {
        if (!getUserSession().isAdmin()) {
            if (getFeatureManager().isFeatureEnabled(Ivr.FEATURE)) {
                throw new PageRedirectException(ManageVoicemail.PAGE);
            } else {
                throw new PageRedirectException(UserRegistrations.PAGE);
            }
        }
    }
}
