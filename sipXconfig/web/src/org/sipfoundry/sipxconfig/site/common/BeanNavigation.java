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
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

@ComponentClass(allowBody = true, allowInformalParameters = true)
public abstract class BeanNavigation extends BaseComponent {

    @Parameter(required = true)
    public abstract BeanWithSettings getBean();

    public abstract void setBean(BeanWithSettings bean);

    @Parameter()
    public abstract Setting getActiveSetting();
}
