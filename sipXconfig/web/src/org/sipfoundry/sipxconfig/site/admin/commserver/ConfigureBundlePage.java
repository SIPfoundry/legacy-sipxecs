/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.feature.Bundle;

public abstract class ConfigureBundlePage extends PageWithCallback {
    public static final String PAGE = "admin/commserver/ConfigureBundlePage";

    public abstract void setBundle(Bundle bundle);

    public abstract Bundle getBundle();

    public void apply() {
    }

    public void ok() {
    }
}
