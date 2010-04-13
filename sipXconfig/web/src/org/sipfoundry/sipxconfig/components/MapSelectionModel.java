/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import java.util.Map;

import org.apache.commons.collections.map.LinkedMap;
import org.apache.tapestry.form.IPropertySelectionModel;

/**
 * Adapt a collection map to a Tapestry select list model. Map keys are not used, map values must
 * be strings.
 */
public class MapSelectionModel implements IPropertySelectionModel {

    private LinkedMap m_map;

    public MapSelectionModel(Map map) {
        setMap(map);
    }

    public MapSelectionModel() {
        // empty - enable bean construction
    }

    /**
     * @param map if instance of commons LinkedMap, use it directly, otherwise copy into a
     *        order-aware map
     */
    public void setMap(Map map) {
        if (map instanceof LinkedMap) {
            m_map = (LinkedMap) map;
        } else {
            m_map = new LinkedMap();
            m_map.putAll(map);
        }
    }

    public int getOptionCount() {
        return m_map.size();
    }

    public Object getOption(int index) {
        return m_map.get(index);
    }

    public String getLabel(int index) {
        return (String) m_map.getValue(index);
    }

    /** based off StringPropertySelectionModel */
    public String getValue(int index) {
        return Integer.toString(index);
    }

    /** based off StringPropertySelectionModel */
    public Object translateValue(String value) {
        int index = Integer.parseInt(value);
        return getOption(index);
    }

    public boolean isDisabled(int index) {
        return false;
    }
}
