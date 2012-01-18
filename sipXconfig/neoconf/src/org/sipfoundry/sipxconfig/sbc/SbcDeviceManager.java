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

import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.bridge.BridgeSbc;
import org.sipfoundry.sipxconfig.commserver.Location;

public interface SbcDeviceManager {
    public static final String CONTEXT_BEAN_NAME = "sbcDeviceManager";

    List<SbcDevice> getSbcDevices();

    SbcDevice getSbcDevice(Integer id);

    void saveSbcDevice(SbcDevice sbc);

    void clear();

    void deleteSbcDevice(Integer id);

    void deleteSbcDevices(Collection<Integer> ids);

    SbcDevice newSbcDevice(SbcDescriptor descriptor);

    Collection<Integer> getAllSbcDeviceIds();

    public BridgeSbc getBridgeSbc(Location location);

    public List<BridgeSbc> getBridgeSbcs();

    public void checkForNewSbcDeviceCreation(SbcDescriptor descriptor);

    public boolean maxAllowedLimitReached(SbcDescriptor model);

    boolean isInternalSbcEnabled();

    BridgeSbc newBridgeSbc(Location location);

    List<Sbc> getSbcsForSbcDeviceId(Integer sbcDeviceId);
}
