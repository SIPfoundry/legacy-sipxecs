/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.site.common;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.sip.SipService;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class ClickToCallPanel extends BaseComponent {
    public abstract String getNumber();

    @InjectObject("spring:sip")
    public abstract SipService getSipService();

    @InjectObject("spring:domainManager")
    public abstract DomainManager getDomainManager();

    @Parameter
    public abstract User getUser();

    @Parameter
    public abstract IValidationDelegate getDelegate();

    public void call() {
        String domain = getDomainManager().getDomain().getName();
        String userAddrSpec = getUser().getAddrSpec(domain);
        String number = getNumber();
        String destAddrSpec = SipUri.fix(number, domain);
        String displayName = "ClickToCall";
        getSipService().sendRefer(getUser(), userAddrSpec, displayName, destAddrSpec);
    }
}
