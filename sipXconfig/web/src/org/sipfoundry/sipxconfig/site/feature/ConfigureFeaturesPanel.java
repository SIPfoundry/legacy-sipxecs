/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.feature;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.contrib.form.IMultiplePropertySelectionRenderer;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.LocalizationUtils;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.site.common.BetterMultiplePropertySelectionRenderer;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class ConfigureFeaturesPanel extends BaseComponent implements PageBeginRenderListener {
    private static final IMultiplePropertySelectionRenderer RENDERER = new BetterMultiplePropertySelectionRenderer();

    @InjectObject("spring:featureManager")
    public abstract FeatureManager getFeatureManager();

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @InjectObject("spring:configManager")
    public abstract ConfigManager getConfigManager();

    public abstract List<LocationFeature> getFeatures();

    public abstract void setFeatures(List<LocationFeature> features);

    @Parameter
    public abstract ICallback getCallback();

    @Parameter(required = true)
    public abstract Location getLocationBean();

    public IMultiplePropertySelectionRenderer getFeaturesRenderer() {
        return RENDERER;
    }

    public IPropertySelectionModel getFeaturesModel() {
        Set<LocationFeature> all = getFeatureManager().getAvailableLocationFeatures(getLocationBean());

        ObjectSelectionModel nakedModel = new ObjectSelectionModel();
        nakedModel.setCollection(all);
        nakedModel.setLabelExpression("id");
        final String feature = "feature.";

        BetterMultiplePropertySelectionRenderer.Model model = new BetterMultiplePropertySelectionRenderer.Model() {
            @Override
            public String getLabel(String rawLabel) {
                return LocalizationUtils.localize(getMessages(), feature + rawLabel);
            }

            @Override
            public String getDescription(String rawLabel) {
                return LocalizationUtils.localize(getMessages(), feature + rawLabel + ".description");
            }
        };
        model.setMessages(getMessages());
        model.setModel(nakedModel);

        return model;
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        if (getFeatures() == null) {
            Set<LocationFeature> enabled = getFeatureManager().getEnabledLocationFeatures(getLocationBean());
            setFeatures(new ArrayList<LocationFeature>(enabled));
        }
    }

    public void saveFeatures() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        HashSet<LocationFeature> enabled = new HashSet<LocationFeature>(getFeatures());
        getFeatureManager().enableLocationFeatures(enabled, getLocationBean());
    }
}
