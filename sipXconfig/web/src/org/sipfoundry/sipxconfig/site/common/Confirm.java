/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.common;

import org.apache.commons.lang.StringEscapeUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;

@ComponentClass(allowBody = false, allowInformalParameters = true)
public abstract class Confirm extends BaseComponent {
    @Parameter(required = true)
    public abstract String getPrompt();

    @Parameter(required = true)
    public abstract IComponent getComponent();

    @Asset(value = "context:/WEB-INF/common/Confirm.script")
    public abstract IAsset getConfirmScript();

    public String getEscapedPrompt() {
        return StringEscapeUtils.escapeJavaScript(getPrompt());
    }
}
