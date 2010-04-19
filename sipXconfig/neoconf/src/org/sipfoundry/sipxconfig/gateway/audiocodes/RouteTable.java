/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;

/**
 * Object representing route table on an AudioCodes PSTN gateway
 */
public abstract class RouteTable extends BeanWithSettings {
    private static final String PREFIX_ANY = "*";
    private static final String STRIP_NONE = "0";

    private Gateway m_gateway;
    private String m_routeId;
    private String m_settingDestManipulation;

    public Gateway getGateway() {
        return m_gateway;
    }

    public void setGateway(Gateway gateway) {
        m_gateway = gateway;
    }

    public String getRouteId() {
        return m_routeId;
    }

    public void setRouteId(String routeId) {
        m_routeId = routeId;
    }

    private String getDestManipulationItem(int itemIndex) {
        String destManipulation = m_gateway.getSettingValue(m_settingDestManipulation);
        if (destManipulation != null) {
            String[] items = destManipulation.split(",");
            if (items.length > itemIndex) {
                return items[itemIndex].trim();
            }
        }
        return null;
    }

    public String getDestPrefix() {
        String item = getDestManipulationItem(0);
        if (item == null) {
            item = PREFIX_ANY;
        }
        return item;
    }

    public String getSrcPrefix() {
        String item = getDestManipulationItem(1);
        if (item == null) {
            item = PREFIX_ANY;
        }
        return item;
    }

    public String getDestLeftStrip() {
        String item = getDestManipulationItem(2);
        if (item == null) {
            item = STRIP_NONE;
        }
        return item;
    }

    public String getDestRightStrip() {
        String item = getDestManipulationItem(3);
        if (item == null) {
            item = STRIP_NONE;
        }
        return item;
    }

    public void setSettingDestManipulation(String settingDestManipulation) {
        m_settingDestManipulation = settingDestManipulation;
    }

    public String getLabel() {
        return "RouteTable";
    }
}
