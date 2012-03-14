/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Lifecycle;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.bean.EvenOdd;
import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.BundleConstraint;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public abstract class ConfigureBundlePage extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "admin/commserver/ConfigureBundlePage";

    public abstract void setBundleId(String bundleId);

    @Persist
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
        FeatureManager fm = getFeatureManager();
        BundleConstraint c = getBundle().getConstraint(f);
        if (c.isLocationDependent(fm, f)) {
            return "global";
        }
        if (c.isSingleLocation(fm, f) && getLocations().size() > 1) {
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

        for (Location l : getLocations()) {
            ByLocation filter = new ByLocation(l);
            Collection<String> subset = CollectionUtils.select(getCellIds(), filter);
            Collection<LocationFeature> features = CollectionUtils.collect(subset, filter);
            Set<LocationFeature> featureSet = new HashSet<LocationFeature>(features);
            getFeatureManager().enableLocationFeatures(featureSet, l);
        }
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
        List<Cell> cells = new ArrayList<Cell>(getLocations().size());
        for (Location l : getLocations()) {
            cells.add(new Cell(getFeature(), l));
        }
        return cells;
    }

    public static class Cell {
        private static final char DELIM = '|';
        private Feature m_feature;
        private Location m_location;

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
