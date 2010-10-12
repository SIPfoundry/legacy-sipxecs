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

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.PageRedirectException;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.valid.ValidatorException;
import org.apache.tapestry.web.WebRequest;
import org.apache.tapestry.web.WebSession;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.security.SipxAuthenticationProcessingFilter;
import org.sipfoundry.sipxconfig.site.user.FirstUser;

public abstract class LoginPage extends PageWithCallback implements PageBeginRenderListener {

    public static final String PAGE = "LoginPage";
    private static final String USER_AGENT = "User-Agent";
    private static final String FIREFOX = "Firefox/";
    private static final String IE7 = "MSIE 7";
    private static final String IE8 = "MSIE 8";
    private static final String CHROME = "Chrome";

    @InjectObject("spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectObject("spring:tapestry")
    public abstract TapestryContext getTapestry();

    @InjectObject(value = "service:tapestry.globals.WebRequest")
    public abstract WebRequest getRequest();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public void pageBeginRender(PageEvent event) {
        // If there are no users in the DB, then redirect to the FirstUser page to make one.
        // For most pages, Border takes care of this check, but LoginPage doesn't have a Border.
        int userCount = getCoreContext().getUsersCount();
        if (userCount == 0) {
            throw new PageRedirectException(FirstUser.PAGE);
        }

        if (StringUtils.isNotEmpty(getRequest().getParameterValue("error"))) {
            getValidator().record(new ValidatorException(getMessages().getMessage("message.loginError")));
            // save original http referer in session so we can later redirect to
            WebSession session = getRequest().getSession(false);
            if (session.getAttribute(SipxAuthenticationProcessingFilter.ORIGINAL_REFERER) == null) {
                session.setAttribute(SipxAuthenticationProcessingFilter.ORIGINAL_REFERER, getRequest().getHeader(
                        "Referer"));
            }
        }

    }

    public boolean isSupportedBrowser() {
        String userAgent = getRequest().getHeader(USER_AGENT);
        return StringUtils.contains(userAgent, FIREFOX) || StringUtils.contains(userAgent, IE7)
                || StringUtils.contains(userAgent, IE8) || StringUtils.contains(userAgent, CHROME);
    }
}
