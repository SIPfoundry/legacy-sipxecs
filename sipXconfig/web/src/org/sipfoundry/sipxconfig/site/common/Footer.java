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
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.Lifecycle;
import org.sipfoundry.sipxconfig.common.VersionInfo;

public abstract class Footer extends BaseComponent {

    @Bean(lifecycle = Lifecycle.PAGE)
    public abstract VersionInfo getVersion();

}
