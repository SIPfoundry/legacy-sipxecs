/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.conference;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.conference.DimDimConference;


@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class DimDimConferenceLink extends BaseComponent {

    @Parameter(required = true)
    public abstract DimDimConference getDimDimConference();
}
