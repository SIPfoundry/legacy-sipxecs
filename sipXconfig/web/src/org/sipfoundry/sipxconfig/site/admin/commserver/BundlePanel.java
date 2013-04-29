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
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Predicate;
import org.apache.commons.collections.Transformer;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Lifecycle;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.bean.EvenOdd;
import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.BundleConstraint;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.InvalidChange;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public abstract class BundlePanel extends BaseComponent implements PageBeginRenderListener {
    private static final String ERR_CLASS = " hilite-user-error";

    private static final String FEATURE = "feature.";

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

    @InjectObject("spring:configManager")
    public abstract ConfigManager getConfigManager();

    public abstract void setLocations(List<Location> locations);

    public abstract List<Location> getLocations();

    public abstract Cell getCell();

    public abstract Set<GlobalFeature> getGlobalFeatures();

    public abstract void setGlobalFeatures(Set<GlobalFeature> features);

    public abstract Set<String> getCellIds();

    public abstract void setCellIds(Set<String> ids);

    public abstract Map<Feature, InvalidChange> getInvalidFeatures();

    public abstract void setInvalidFeatures(Map<Feature, InvalidChange> rows);

    @Bean(lifecycle = Lifecycle.RENDER)
    public abstract EvenOdd getRowClass();

    public String getCellClass() {
        String cls = "activeColumnValue";
        Map<Feature, InvalidChange> errs = getInvalidFeatures();
        if (errs != null) {
            InvalidChange invalidItem = errs.get(getFeature());
            if (invalidItem != null) {
                if (getFeature() instanceof GlobalFeature) {
                    cls += ERR_CLASS;
                } else if (invalidItem.getLocation() == null) {
                    cls += ERR_CLASS;
                } else if (invalidItem.getLocation().getId() == getLocationObject().getId()) {
                    cls += ERR_CLASS;
                }
            }
        }

        return cls;
    }

    public Collection<Feature> getSortedFeatures() {
        Map<String, Feature> sortedMap = new TreeMap<String, Feature>();
        for (Feature feature : getBundle().getFeatures()) {
            String label = getMessages().getMessage(FEATURE + feature.getId());
            sortedMap.put(label.toLowerCase(), feature);
        }

        return sortedMap.values();
    }

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Asset(value = "context:/WEB-INF/admin/commserver/BundlePanel.script")
    public abstract IAsset getScript();

    public String getConstraintBlockId() {
        Feature f = getFeature();
        if (f instanceof GlobalFeature) {
            return "global";
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
            locations = new ArrayList<Location>(getConfigManager().getRegisteredLocations());
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

    public void validate() {
        FeatureChangeRequest request = buildFeatureChangeRequest();
        FeatureChangeValidator validator = new FeatureChangeValidator(getFeatureManager(), request);
        getFeatureManager().validateFeatureChange(validator);
        recordAnyInvalidChanges(validator);
        rebuildForm(request);
    }

    private void recordAnyInvalidChanges(FeatureChangeValidator validator) {
        for (InvalidChange err : validator.getInvalidChanges()) {
            getValidator().record(localize(err.getMessage()), getMessages());
        }
        setInvalidFeatures(indexByFeature(validator.getInvalidChanges()));
    }

    UserException localize(UserException err) {
        Object[] params = err.getRawParams();
        if (params != null && params.length > 0) {
            for (int i = 0; i < params.length; i++) {
                if (params[i] instanceof Feature) {
                    params[i] = getMessages().getMessage(FEATURE + params[i]);
                }
            }
        }

        return err;
    }

    void rebuildForm(FeatureChangeRequest request) {
        setGlobalFeatures(request.getEnable());
        Set<String> cellIds = new HashSet<String>();
        List<Location> locations = getLocations();
        for (Location l : locations) {
            Set<LocationFeature> enabled = request.getEnableByLocation().get(l);
            for (LocationFeature f : enabled) {
                cellIds.add(Cell.encodeId(f, l));
            }
        }
        setCellIds(cellIds);
    }

    public void save() {
        FeatureChangeRequest request = buildFeatureChangeRequest();
        FeatureChangeValidator validator = new FeatureChangeValidator(getFeatureManager(), request);
        try {
            getFeatureManager().applyFeatureChange(validator);
        } catch (UserException e) {
            recordAnyInvalidChanges(validator);
            getValidator().record(e, getMessages());
        }
        rebuildForm(request);
    }

    @SuppressWarnings("unchecked")
    public FeatureChangeRequest buildFeatureChangeRequest() {
        Map<Location, Set<LocationFeature>> byLocation = new HashMap<Location, Set<LocationFeature>>();
        // location
        for (Location l : getLocations()) {
            ByLocation filter = new ByLocation(l, getBundle());
            Collection<String> subset = CollectionUtils.select(getCellIds(), filter);
            Collection<LocationFeature> selected = CollectionUtils.collect(subset, filter);
            Set<LocationFeature> featureSet = new HashSet<LocationFeature>(selected);
            byLocation.put(l, featureSet);
        }

        FeatureChangeRequest req = FeatureChangeRequest.byBundle(getBundle(), getGlobalFeatures(), byLocation);
        return req;
    }

    static class ByLocation implements Transformer, Predicate {
        private String m_locationId;
        private Bundle m_bundle;

        public ByLocation(Location l, Bundle bundle) {
            m_locationId = l.getId().toString();
            m_bundle = bundle;
        }

        @Override
        public Object transform(Object arg0) {
            String[] ids = Cell.decodeId(arg0.toString());
            Feature f = m_bundle.getFeature(ids[0]);
            return f;
        }

        @Override
        public boolean evaluate(Object arg0) {
            String[] ids = Cell.decodeId(arg0.toString());
            Feature f = m_bundle.getFeature(ids[0]);
            return (ids[1].equals(m_locationId) && f != null);
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

    public Map<Feature, InvalidChange> indexByFeature(List<InvalidChange> items) {
        Map<Feature, InvalidChange> index = new HashMap<Feature, InvalidChange>(items.size());
        for (InvalidChange item : items) {
            index.put(item.getFeature(), item);

        }
        return index;
    }
}
