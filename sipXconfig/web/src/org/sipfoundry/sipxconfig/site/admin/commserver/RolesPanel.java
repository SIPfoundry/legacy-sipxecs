/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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

import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.feature.FeatureManager;

public abstract class RolesPanel extends BaseComponent implements PageBeginRenderListener {

    public abstract Bundle getBundle();

    public abstract String getSelectedBundleId();

    public abstract Feature getFeature();

    public abstract Object getToken();

    public abstract List<Bundle> getBundles();

    public abstract void setBundles(List<Bundle> bundles);

    public abstract String getDelim();

    @InjectObject("spring:featureManager")
    public abstract FeatureManager getFeatureManager();

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    public boolean isSingleServer() {
        return getLocationsManager().getLocations().length == 1;
    }

    public void pageBeginRender(PageEvent event) {
        List<Bundle> bundles = getBundles();
        if (bundles == null) {
            bundles = getFeatureManager().getBundles();
            setBundles(bundles);
        }
    }

    public void enableOnPrimary(IRequestCycle cycle) {
        getFeatureManager().enableBundleOnPrimary(getBundle());
    }

    public IPage configure(IRequestCycle cycle) {
        ConfigureBundlePage page = (ConfigureBundlePage) cycle.getPage(ConfigureBundlePage.PAGE);
        page.setBundleId(getBundle().getId());
        page.setReturnPage(getPage());
        return page;
    }

    public IPrimaryKeyConverter getBundleConverter() {
        return new IPrimaryKeyConverter() {
            @Override
            public Object getPrimaryKey(Object value) {
                return ((Bundle) value).getId();
            }

            @Override
            public Object getValue(Object primaryKey) {
                return getFeatureManager().getBundle(primaryKey.toString());
            }
        };
    }

    /**
     * Just a dummy converter, we don't care about [de]serializing features
     */
    public IPrimaryKeyConverter getFeatureConverter() {
        return new IPrimaryKeyConverter() {
            @Override
            public Object getPrimaryKey(Object value) {
                return ((Feature) value).getId();
            }

            @Override
            public Object getValue(Object primaryKey) {
                return getBundle().getFeature(primaryKey.toString());
            }
        };
    }
}
