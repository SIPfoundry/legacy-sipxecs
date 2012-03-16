/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.apache;

import org.sipfoundry.sipxconfig.feature.LocationFeature;

public interface ApacheManager {

    public static final LocationFeature FEATURE = new LocationFeature("apache");

    public void avoidCheckstyleError();

}
