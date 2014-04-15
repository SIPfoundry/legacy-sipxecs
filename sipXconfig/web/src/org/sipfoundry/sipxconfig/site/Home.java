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

import java.net.URL;

import org.apache.tapestry.PageRedirectException;
import org.apache.tapestry.RedirectException;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectState;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.web.WebRequest;
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
        // TODO this implementation is wrong, redirection should not be done from Tapestry but
        // at Spring level in SipxSimpleUrlAuthenticationSuccessHandler!
        // Right now we're redirecting twice on success authentication, once to Home and again to unite web
        if (!getUserSession().isAdmin()) {
            Setting adminSettings = getAdminContext().getSettings().getSettings();
            Setting oldPortal = adminSettings.getSetting("user-portal/old-portal");
            if (oldPortal != null) {
                boolean oldPortalEnabled = (Boolean) oldPortal.getTypedValue();
                if (!oldPortalEnabled) {
                    throw new RedirectException(getRedirectUrl("/unitelite"));
                }
            }
            if (getFeatureManager().isFeatureEnabled(Ivr.FEATURE)) {
                throw new PageRedirectException(ManageVoicemail.PAGE);
            }
            throw new PageRedirectException(UserRegistrations.PAGE);
        }
    }

    protected String getRedirectUrl(String suffix) {
        try {
            WebRequest request = getRequest();
            String referer = request.getHeader("Referer");
            URL refererUrl = new URL(referer);
            StringBuilder url = new StringBuilder();
            String protocol = refererUrl.getProtocol();
            url.append(protocol);
            url.append("://");
            url.append(refererUrl.getHost());
            int port = refererUrl.getPort();
            if (port > 0
                    && ((protocol.equalsIgnoreCase("http") && port != 80)
                    || (protocol.equalsIgnoreCase("https") && port != 443))) {
                url.append(':');
                url.append(port);
            }
            url.append(suffix);
            return url.toString();
        } catch (Exception ex) {
            return suffix;
        }
    }

}
