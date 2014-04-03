/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.web.WebRequest;

public abstract class SipxBasePage extends BasePage {
    protected static final String FIREFOX = "Firefox/";
    protected static final String IE7 = "MSIE 7";
    protected static final String IE8 = "MSIE 8";
    protected static final String IE9 = "MSIE 9";
    protected static final String IE10 = "MSIE 10";
    protected static final String IE11 = "rv:11.";
    protected static final String CHROME = "Chrome";
    protected static final String SAFARI = "Safari/";
    protected static final String OPERA_OLD = "Opera/";
    protected static final String OPERA_OLD_VERSION_10_PLUS = "Version/1";
    //Starting with version 15 Opera browsers uses Chrome engine and the signature is changed
    protected static final String OPERA_NEW = "OPR/";

    private static final String USER_AGENT = "User-Agent";

    @InjectObject(value = "service:tapestry.globals.WebRequest")
    public abstract WebRequest getRequest();

    public String getBorderTitle() {
        String borderTitle = getPage().getMessages().getMessage("title");
        if (borderTitle.equals("[TITLE]")) {
            // display nothing
            return "&nbsp;";
        }
        return borderTitle;
    }

    public boolean isSupportedBrowser() {
        String userAgent = getBrowser();
        return StringUtils.contains(userAgent, FIREFOX) || StringUtils.contains(userAgent, IE7)
                || StringUtils.contains(userAgent, IE8) || StringUtils.contains(userAgent, IE9)
                || StringUtils.contains(userAgent, IE10) || StringUtils.contains(userAgent, CHROME)
                || StringUtils.contains(userAgent, SAFARI) || StringUtils.contains(userAgent, OPERA_OLD)
                || StringUtils.contains(userAgent, OPERA_NEW) || StringUtils.contains(userAgent, IE11);
    }

    protected String getBrowser() {
        return getRequest().getHeader(USER_AGENT);
    }
}
