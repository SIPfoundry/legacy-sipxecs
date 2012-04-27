/**
 *
 *
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
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Predicate;
import org.apache.commons.collections.Transformer;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Lifecycle;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.bean.EvenOdd;
import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.BundleConstraint;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public abstract class BundlePanel extends BaseComponent implements PageBeginRenderListener {
    public abstract void setBundleId(String bundleId);

    @Parameter
    public abstract String getBundleId();

    public abstract Bundle getBundle();

    public abstract void setBundle(Bundle bundle);

    public abstract Feature getFeature();

    public abstract Location getLocationObject();

    @InjectObject("spring:featureManager")
    public abstract FeatureManager getFeatureManager();

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    public abstract void setLocations(List<Location> locations);

    public abstract List<Location> getLocations();

    public abstract Cell getCell();

    public abstract Set<GlobalFeature> getGlobalFeatures();

    public abstract void setGlobalFeatures(Set<GlobalFeature> features);

    public abstract Set<String> getCellIds();

    public abstract void setCellIds(Set<String> ids);

    @Bean(lifecycle = Lifecycle.RENDER)
    public abstract EvenOdd getRowClass();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public String getConstraintBlockId() {
        Feature f = getFeature();
        if (f instanceof GlobalFeature) {
            return "global";
        }

        FeatureManager fm = getFeatureManager();
        BundleConstraint c = getBundle().getConstraint(f);
        Collection<Location> candidates = c.getApplicableLocations(fm, f, getLocations());
        if (c.isSingleLocation(fm, f) && candidates.size() > 1) {
            return "pickOne";
        }

        return "pickAny";
    }

    public void pageBeginRender(PageEvent event) {
        Bundle bundle = getBundle();
        if (bundle == null) {
            bundle = getFeatureManager().getBundle(getBundleId());
            setBundle(bundle);
        }

        List<Location> locations = getLocations();
        if (locations == null) {
            locations = getLocationsManager().getLocationsList();
            setLocations(locations);
        }

        Set<GlobalFeature> global = getGlobalFeatures();
        if (global == null) {
            global = getFeatureManager().getEnabledGlobalFeatures();
            setGlobalFeatures(global);
        }

        Set<String> cellIds = getCellIds();
        if (cellIds == null) {
            cellIds = new HashSet<String>();
            for (Location l : locations) {
                for (LocationFeature f : getFeatureManager().getEnabledLocationFeatures(l)) {
                    cellIds.add(Cell.encodeId(f, l));
                }
            }
            setCellIds(cellIds);
        }
    }

    @SuppressWarnings("unchecked")
    public void save() {
        getFeatureManager().enableGlobalFeatures(getGlobalFeatures());

        Set<LocationFeature> bundleFeatures = getBundleLocationFeatures();
        for (Location l : getLocations()) {
            ByLocation filter = new ByLocation(l);
            Collection<String> subset = CollectionUtils.select(getCellIds(), filter);
            Collection<LocationFeature> selected = CollectionUtils.collect(subset, filter);
            Collection<LocationFeature> unselected = CollectionUtils.disjunction(bundleFeatures, selected);
            Set<LocationFeature> enabledFeatures = getFeatureManager().getEnabledLocationFeatures(l);
            enabledFeatures.addAll(selected);
            enabledFeatures.removeAll(unselected);
            Set<LocationFeature> featureSet = new HashSet<LocationFeature>(enabledFeatures);
            if (featureSet.contains(null)) {
                featureSet.remove(null);
            }
            getFeatureManager().enableLocationFeatures(featureSet, l);
        }
    }

    private Set<LocationFeature> getBundleLocationFeatures() {
        Set<LocationFeature> featureSet = new HashSet<LocationFeature>();
        for (Feature feature : getBundle().getFeatures()) {
            if (feature instanceof LocationFeature) {
                featureSet.add((LocationFeature) feature);
            }
        }
        return featureSet;
    }

    class ByLocation implements Transformer, Predicate {
        private String m_locationId;

        public ByLocation(Location l) {
            m_locationId = l.getId().toString();
        }

        @Override
        public Object transform(Object arg0) {
            String[] ids = Cell.decodeId(arg0.toString());
            Feature f = getBundle().getFeature(ids[0]);
            return f;
        }

        @Override
        public boolean evaluate(Object arg0) {
            String[] ids = Cell.decodeId(arg0.toString());
            return ids[1].equals(m_locationId);
        }
    }

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

    public boolean isGlobalFeatureEnabled() {
        return getGlobalFeatures().contains(getFeature());
    }

    public void setGlobalFeatureEnabled(boolean enabled) {
        if (enabled) {
            getGlobalFeatures().add((GlobalFeature) getFeature());
        } else {
            getGlobalFeatures().remove((GlobalFeature) getFeature());
        }
    }

    public Cell getPickedSingleCell() {
        for (Location l : getLocations()) {
            String id = Cell.encodeId(getFeature(), l);
            if (getCellIds().contains(id)) {
                return new Cell(getFeature(), l);
            }
        }
        return null;
    }

    public void setPickedSingleCell(Cell c) {
        getCellIds().add(c.getId());
    }

    public boolean isCellPickedFromAnyInRow() {
        String id = getCell().getId();
        boolean enabled = getCellIds().contains(id);
        return enabled;
    }

    public void setCellPickedFromAnyInRow(boolean enabled) {
        if (enabled) {
            getCellIds().add(getCell().getId());
        } else {
            getCellIds().remove(getCell().getId());
        }
    }

    public Collection<Cell> getCells() {
        Feature f = getFeature();
        FeatureManager fm = getFeatureManager();
        BundleConstraint c = getBundle().getConstraint(f);
        Collection<Location> validLocations = c.getApplicableLocations(fm, f, getLocations());
        Collection<Integer> valid = CollectionUtils.collect(validLocations, new BeanWithId.BeanToId());
        List<Cell> cells = new ArrayList<Cell>(valid.size());
        for (Location l : getLocations()) {
            Cell cell = new Cell(getFeature(), l);
            cell.m_enabled = valid.contains(l.getId());
            cells.add(cell);

        }
        return cells;
    }

    public static class Cell {
        private static final char DELIM = '|';
        private Feature m_feature;
        private Location m_location;
        private boolean m_enabled;

        Cell(Feature f, Location l) {
            m_feature = f;
            m_location = l;
        }

        public String getId() {
            return encodeId(m_feature, m_location);
        }

        public static String encodeId(Feature f, Location l) {
            return f.getId() + DELIM + l.getId();
        }

        public static String[] decodeId(String cellId) {
            int i = cellId.indexOf(DELIM);
            return new String[] {
                cellId.substring(0, i), cellId.substring(i + 1)
            };
        }

        public Feature getFeature() {
            return m_feature;
        }

        public Location getLocation() {
            return m_location;
        }

        public boolean isEnabled() {
            return m_enabled;
        }
    }

    Location getLocation(String id) {
        int i = Integer.parseInt(id);
        for (Location l : getLocations()) {
            if (l.getId() == i) {
                return l;
            }
        }
        return null;
    }

    public IPrimaryKeyConverter getCellConverter() {
        return new IPrimaryKeyConverter() {
            @Override
            public Object getPrimaryKey(Object value) {
                return ((Cell) value).getId();
            }

            @Override
            public Object getValue(Object primaryKey) {
                String[] ids = Cell.decodeId(primaryKey.toString());
                Feature f = getBundle().getFeature(ids[0]);
                Location l = getLocation(ids[1]);
                return new Cell(f, l);
            }
        };
    }
}
