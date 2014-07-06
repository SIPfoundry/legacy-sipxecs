/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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

import org.apache.commons.lang.ObjectUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.components.selection.AdaptedSelectionModel;
import org.sipfoundry.sipxconfig.components.selection.OptionAdapter;

public abstract class LocationSelection extends BaseComponent {

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @InjectObject("spring:tapestry")
    public abstract TapestryContext getTapestry();

    public abstract void setServer(Location location);

    @Parameter(required = true)
    public abstract Location getServer();

    public abstract void setSelectedAction(IActionListener selectedAction);

    public abstract IActionListener getSelectedAction();

    public abstract void setLocations(List<Location> locations);

    @Parameter(required = true)
    public abstract List<Location> getLocations();

    @Override
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        Location location = getServer();
        if (location != null) {
            setSelectedAction(new LocationAction(location));
        }

        if (location == null) {
            setSelectedAction(null);
        }

        super.renderComponent(writer, cycle);
        if (TapestryUtils.isRewinding(cycle, this) && TapestryUtils.isValid(this)) {
            triggerAction(cycle);
        }
    }

    private void triggerAction(IRequestCycle cycle) {
        IActionListener a = getSelectedAction();
        LocationAction action;
        if (a != null) {
            if (!(a instanceof LocationAction)) {
                return;
            }

            action = (LocationAction) a;
            Location location = action.getSelectedLocation();
            if (location != null) {
                action.setId(location.getId());
            }
        } else {
            // no item is selected
            action = new LocationAction(null);
            action.setId(null);
        }
        action.actionTriggered(this, cycle);
    }

    public IPropertySelectionModel getLocationsModel() {
        Collection<OptionAdapter<Location>> options = new ArrayList<OptionAdapter<Location>>();
        for (Location location : getLocations()) {
            LocationAction adapter = new LocationAction(location);
            options.add(adapter);
        }
        AdaptedSelectionModel locationSelectionModel = new AdaptedSelectionModel();
        locationSelectionModel.setCollection(options);
        return getTapestry().instructUserToSelect(locationSelectionModel, getMessages());
    }

    private class LocationAction extends LocationAdapter {
        public LocationAction(Location location) {
            super(location);
        }

        public void actionTriggered(IComponent component, final IRequestCycle cycle) {
            setServer(getSelectedLocation());
        }

        @Override
        public Object getValue(Object option, int index) {
            return this;
        }

        @Override
        public String squeezeOption(Object option, int index) {
            return getSelectedLocation().getId().toString();
        }

        @Override
        public boolean equals(Object obj) {
            return ObjectUtils.equals(this.getSelectedLocation(), ((LocationAction) obj).getSelectedLocation());
        }

        @Override
        public int hashCode() {
            return this.getSelectedLocation().hashCode();
        }
    }
}
