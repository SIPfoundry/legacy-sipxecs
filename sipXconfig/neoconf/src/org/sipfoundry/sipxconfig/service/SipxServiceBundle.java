/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import org.apache.commons.collections.Predicate;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.device.Model;

public class SipxServiceBundle implements Model {
    private final String m_name;
    private boolean m_autoEnable;
    private String m_modelId;
    private boolean m_onlyPrimary;
    private boolean m_onlyRemote;
    private int m_min;
    private int m_max;
    private boolean m_resetAffectLocation;

    public SipxServiceBundle(String name) {
        m_name = name;
    }

    public String getName() {
        return m_name;
    }

    @Override
    public int hashCode() {
        return m_name.hashCode();
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (obj == null || getClass() != obj.getClass()) {
            return false;
        }
        SipxServiceBundle bundle = (SipxServiceBundle) obj;
        return m_name.equals(bundle.m_name);
    }

    public void setAutoEnable(boolean autoEnable) {
        m_autoEnable = autoEnable;
    }

    public boolean isAutoEnable() {
        return m_autoEnable;
    }

    public void setBeanName(String name) {
        m_modelId = name;
    }

    public String getModelId() {
        return m_modelId;
    }

    public void setOnlyPrimary(boolean onlyPrimary) {
        m_onlyPrimary = onlyPrimary;
    }

    public void setOnlyRemote(boolean onlyRemote) {
        m_onlyRemote = onlyRemote;
    }

    public boolean canRunOn(Location location) {
        if (location.isPrimary()) {
            return !m_onlyRemote;
        }
        return !m_onlyPrimary;
    }

    /**
     * Decides if bundles should be enabled automatically on a location
     *
     * On primary location all auto-enabled bundles are OK as long as the can run on primary. On
     * remote locations bundles has to be restricted to remoteOnly if we want to autoEnable it.
     * For example we do not want v-mail automatically enabled on remote location.
     *
     * @param location server on which we want to run a bundle
     * @return true is the bundle should be automatically enabled
     */
    public boolean autoEnableOn(Location location) {
        if (!isAutoEnable()) {
            return false;
        }
        return location.isPrimary() ? !m_onlyRemote : m_onlyRemote;
    }

    public void setMin(int min) {
        m_min = min;
    }

    public void setMax(int max) {
        m_max = max;
    }

    public boolean isResetAffectLocation() {
        return m_resetAffectLocation;
    }

    public void setResetAffectLocation(boolean resetAffectLocation) {
        m_resetAffectLocation = resetAffectLocation;
    }

    /**
     * Throws exception if new count
     *
     * @param newCount
     */
    public void verifyCount(int newCount) {
        if (newCount < m_min) {
            throw new TooFewBundles(this, m_min);
        }
        if (newCount > m_max && m_max > 0) {
            throw new TooManyBundles(this, m_max);
        }
    }

    static class BundleException extends UserException {
        public BundleException(String msg, SipxServiceBundle bundle, int count) {
            super(msg, "&bundle." + bundle.getName(), count);
        }
    }

    static class TooManyBundles extends BundleException {
        public TooManyBundles(SipxServiceBundle bundle, int max) {
            super("&msg.tooManyBundles", bundle, max);
        }
    }

    static class TooFewBundles extends BundleException {
        public TooFewBundles(SipxServiceBundle bundle, int min) {
            super("&msg.tooFewBundles", bundle, min);
        }
    }

    public static class CanRunOn implements Predicate {
        private final Location m_location;

        public CanRunOn(Location location) {
            m_location = location;
        }

        public boolean evaluate(Object item) {
            SipxServiceBundle bundle = (SipxServiceBundle) item;
            return bundle.canRunOn(m_location);
        }
    }
}
