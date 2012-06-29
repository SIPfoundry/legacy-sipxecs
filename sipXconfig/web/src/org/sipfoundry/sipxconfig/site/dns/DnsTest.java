/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.dns;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.dns.DnsTestContext;

public abstract class DnsTest extends BaseComponent {

    @Persist
    @InitialValue(value = "literal:localhost")
    public abstract String getDnsServer();

    @Persist
    public abstract String getResults();
    public abstract void setResults(String results);

    @Asset("/images/bullet_error.png")
    public abstract IAsset getDnsErrorImage();

    @Asset("/images/bullet_green.png")
    public abstract IAsset getDnsSuccessImage();

    @InjectObject(value = "spring:dnsTestContext")
    public abstract DnsTestContext getDnsTestContext();

    public abstract void setShowDetailedHelp(boolean toggle);

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public boolean isResultsNotBlank() {
        return StringUtils.isNotBlank(getResults());
    }

    public void execute() {
        String missingRecords = getDnsTestContext().missingRecords(getDnsServer());
        setResults(missingRecords);
        if (StringUtils.isBlank(missingRecords)) {
            TapestryUtils.recordSuccess(this, getMessages().getMessage("validationDns.labelSuccess"));
        }
    }
}
