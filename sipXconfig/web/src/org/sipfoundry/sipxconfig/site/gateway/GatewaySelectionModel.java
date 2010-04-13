/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.gateway;

import java.util.Iterator;
import java.util.List;

import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.gateway.Gateway;

/**
 * GatewaySelectionModel
 */
public class GatewaySelectionModel implements IPropertySelectionModel {
    private List m_gateways;

    /**
     * Bean style initialization; you can only call it once
     *
     * @param gateways list of gateways for the model
     */
    public void setGateways(List gateways) {
        if (m_gateways != null) {
            throw new IllegalStateException("Object has been already initialized");
        }
        m_gateways = gateways;
    }

    public int getOptionCount() {
        return m_gateways.size();
    }

    public Object getOption(int index) {
        return m_gateways.get(index);
    }

    public String getLabel(int index) {
        Gateway gateway = (Gateway) getOption(index);
        return gateway.getName();
    }

    public String getValue(int index) {
        Gateway gateway = (Gateway) getOption(index);
        return gateway.getId().toString();
    }

    public Object translateValue(String value) {
        if (value == null) {
            return null;
        }
        Integer id = Integer.decode(value);
        for (Iterator i = m_gateways.iterator(); i.hasNext();) {
            Gateway g = (Gateway) i.next();
            if (g.getId().equals(id)) {
                return g;
            }
        }
        throw new IllegalArgumentException(value + " is not a valid gateway id");
    }

    public boolean isDisabled(int index) {
        return false;
    }
}
