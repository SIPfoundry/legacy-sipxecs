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

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Message;
import org.apache.tapestry.annotations.Parameter;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class EnabledColumn extends BaseComponent {
    @Parameter(required = true)
    public abstract boolean getValue();

    @Parameter(required = false)
    public abstract String getKeyPrefix();

    @Message(value = "enabled")
    public abstract String getDefaultOnLabel();

    @Message(value = "disabled")
    public abstract String getDefaultOffLabel();

    public String getOnLabel() {
        if (getKeyPrefix() != null) {
            return getLabel(".on");
        }
        return getDefaultOnLabel();
    }

    public String getOffLabel() {
        if (getKeyPrefix() != null) {
            return getLabel(".off");
        }
        return getDefaultOffLabel();
    }

    private String getLabel(String suffix) {
        return getContainer().getMessages().getMessage(getKeyPrefix() + suffix);
    }
}
