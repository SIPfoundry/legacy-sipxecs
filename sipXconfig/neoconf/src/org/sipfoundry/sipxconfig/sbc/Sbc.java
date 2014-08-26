/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.sbc;

import java.util.Arrays;
import java.util.Collection;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.gateway.SipTrunk;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversal;
import org.sipfoundry.sipxconfig.systemaudit.SystemAuditable;

public abstract class Sbc extends BeanWithId implements DeployConfigOnEdit, SystemAuditable {

    private boolean m_enabled;

    private SbcRoutes m_routes;

    private SbcDevice m_sbcDevice;

    private String m_address;

    private int m_port = SipTrunk.DEFAULT_PORT;

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public SbcRoutes getRoutes() {
        return m_routes;
    }

    public void setRoutes(SbcRoutes routes) {
        m_routes = routes;
    }

    public SbcDevice getSbcDevice() {
        return m_sbcDevice;
    }

    public void setSbcDevice(SbcDevice sbcDevice) {
        m_sbcDevice = sbcDevice;
    }

    public String getAddress() {
        return m_address;
    }

    public void setAddress(String address) {
        m_address = address;
    }

    public void setPort(int port) {
        m_port = port;
    }

    public int getPort() {
        return m_port;
    }

    public String getRoute() {
        StringBuilder route = new StringBuilder(m_address);
        if (m_port > 0 && m_port != SipTrunk.DEFAULT_PORT) {
            route.append(':');
            route.append(getPort());
        }
        return route.toString();
    }

    /**
     * Called when SBC device asociated with this SBC gets deleted.
     *
     * @return true if SBC should be deleted as well
     */
    public boolean onDeleteSbcDevice() {
        return true;
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Arrays.asList((Feature) DialPlanContext.FEATURE, (Feature) NatTraversal.FEATURE);
    }

    @Override
    public String getEntityIdentifier() {
        return getAddress();
    }

    @Override
    public String getConfigChangeType() {
        return Sbc.class.getSimpleName();
    }
}
