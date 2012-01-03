/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.feature;


import java.util.Set;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.LocalizedOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class ConfigureFeaturesPanel extends BaseComponent implements PageBeginRenderListener {

    @InjectObject("spring:featureManager")
    public abstract FeatureManager getFeatureManager();

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    public abstract Set<LocationFeature> getFeatures();

    public abstract void setFeatures(Set<LocationFeature> features);

    @Parameter
    public abstract ICallback getCallback();

    @Parameter(required = true)
    public abstract Location getLocationBean();

    public IPropertySelectionModel getFeaturesModel() {
        Set<LocationFeature> all = getFeatureManager().getAvailableLocationFeatures(getLocationBean());

        ObjectSelectionModel nakedModel = new ObjectSelectionModel();
        nakedModel.setCollection(all);
        nakedModel.setLabelExpression("id");

        LocalizedOptionModelDecorator model = new LocalizedOptionModelDecorator();
        model.setMessages(getMessages());
        model.setResourcePrefix("bundle.");
        model.setModel(nakedModel);

        return model;
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        if (getFeatures() == null) {
            setFeatures(getFeatureManager().getEnabledLocationFeatures(getLocationBean()));
        }
    }

    public void saveFeatures() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        getFeatureManager().enableLocationFeatures(getFeatures(), getLocationBean());
    }
}
