/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.nattraversal;

import org.sipfoundry.sipxconfig.feature.GlobalFeature;

public interface NatTraversal {
    public static final GlobalFeature FEATURE = new GlobalFeature("natTraversal");

    public NatSettings getSettings();

    public void saveSettings(NatSettings settings);
}
