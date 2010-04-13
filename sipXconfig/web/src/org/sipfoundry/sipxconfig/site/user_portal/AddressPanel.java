/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.user_portal;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.phonebook.Address;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class AddressPanel extends BaseComponent {

    @Parameter(required = true)
    public abstract Address getAddr();

    @Parameter(required = true)
    public abstract String getPanelLabel();

    @Parameter(required = false, defaultValue = "false")
    public abstract boolean isOffice();
}
