/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.dialplan.sbc;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.Sbc;

public abstract class SbcPanel extends BaseComponent {
    @Parameter(required = true)
    public abstract Sbc getSbc();
}
