/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.device;

import java.util.HashSet;
import java.util.Set;

import org.apache.commons.lang.enums.Enum;
import org.sipfoundry.sipxconfig.common.EnumUserType;

/**
 * For devices that need to distinguish version can enumerate versions
 */
public final class DeviceVersion extends Enum implements FeatureProvider {
    private final String m_versionId;
    private final String m_vendorId;
    private Set<String> m_supportedFeatures = new HashSet<String>();

    public DeviceVersion(String vendorId, String versionId) {
        super(vendorId + versionId);
        m_vendorId = vendorId;
        m_versionId = versionId;
    }

    public String getVendorId() {
        return m_vendorId;
    }

    public String getVersionId() {
        return m_versionId;
    }

    public void setSupportedFeatures(Set<String> supportedFeatures) {
        m_supportedFeatures = supportedFeatures;
    }

    public Set<String> getSupportedFeatures() {
        return m_supportedFeatures;
    }

    public void addSupportedFeature(String feature) {
        m_supportedFeatures.add(feature);
    }

    public boolean isSupported(String feature) {
        return m_supportedFeatures.contains(feature);
    }

    /** For hibernate mapping */
    public static class Type extends EnumUserType {
        public Type() {
            super(DeviceVersion.class);
        }
    }

    /**
     * Decode version string back into enum
     */
    public static DeviceVersion getDeviceVersion(String name) {
        DeviceVersion version = (DeviceVersion) Enum.getEnum(DeviceVersion.class, name);
        return version;
    }
}
