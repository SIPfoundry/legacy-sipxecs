/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * 
 */
package org.sipfoundry.sipxconfig.admin.dialplan.sbc;

import java.util.Collection;
import java.util.List;

public interface SbcDeviceManager {
    List<SbcDevice> getSbcDevices();

    SbcDevice getSbcDevice(Integer id);

    void storeSbcDevice(SbcDevice sbc);

    void clear();

    void deleteSbcDevice(Integer id);

    void deleteSbcDevices(Collection<Integer> ids);

    SbcDevice newSbcDevice(SbcDescriptor descriptor);

    Collection<Integer> getAllSbcDeviceIds();
}
