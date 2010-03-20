/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.sipfoundry.sipxconfig.admin.DnsTestContext;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

public abstract class DnsTestPage extends SipxBasePage {

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract boolean getProvideDns();

    @Persist
    public abstract boolean getShowTestResults();
    public abstract void setShowTestResults(boolean showTestResults);

    @Persist
    public abstract boolean isShowDetailedHelp();

    @Asset("/images/bullet_error.png")
    public abstract IAsset getDnsErrorImage();

    @Asset("/images/bullet_green.png")
    public abstract IAsset getDnsSuccessImage();

    @InjectObject(value = "spring:dnsTestContext")
    public abstract DnsTestContext getDnsTestContext();


    public void execute() {
        getDnsTestContext().execute(getProvideDns());
    }
}
