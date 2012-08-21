/**
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureManager;

public abstract class LocationsPage extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "admin/commserver/LocationsPage";

    public abstract Bundle getCurrentBundle();

    @Persist
    @InitialValue("literal:locations")
    public abstract String getTab();

    public abstract void setTab(String id);

    public abstract Collection<Bundle> getBundles();

    public abstract void setBundles(Collection<Bundle> bundles);

    @InjectObject("spring:featureManager")
    public abstract FeatureManager getFeatureManager();

    public void editLocations() {
        setTab("locations");
    }

    public void editBundle(String bundleId) {
        setTab(bundleId);
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        if (getBundles() == null) {
            List<Bundle> bundles = new ArrayList<Bundle>(getFeatureManager().getBundles());
            // start at end because we're removing items while iterating
            for (int i = bundles.size() - 1; i >= 0; i--) {
                if (bundles.get(i).getFeatures().size() == 0) {
                    bundles.remove(i);
                }
            }

            setBundles(bundles);
        }
    }
}
