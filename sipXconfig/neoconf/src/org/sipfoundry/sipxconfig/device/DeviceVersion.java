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
import org.apache.commons.lang.enums.Enum;
import org.sipfoundry.sipxconfig.common.EnumUserType;

/**
 * For devices that need to distinguish version can enumerate versions
 */
public final class DeviceVersion extends Enum {    
    private String m_versionId;
    private String m_vendorId;
    
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
