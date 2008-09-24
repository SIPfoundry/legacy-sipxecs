/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.gateway;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.gateway.GatewayCallerAliasInfo;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class GatewayCallerAliasInfoPanel extends BaseComponent implements
        PageBeginRenderListener {

    @Parameter(required = true)
    public abstract GatewayCallerAliasInfo getGcai();

    @Persist
    public abstract boolean isAdvanced();

    public void pageBeginRender(PageEvent event) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        if (getGcai().isEnableCallerId() && getGcai().getCallerId() == null) {
            TapestryUtils.getValidator(getPage())
                    .record(
                            new ValidatorException(getMessages().getMessage(
                                    "message.mandatoryCallerId")));
            return;
        }
    }
}
