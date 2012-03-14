/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
