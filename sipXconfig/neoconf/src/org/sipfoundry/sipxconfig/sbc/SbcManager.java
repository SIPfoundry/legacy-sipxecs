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

import org.sipfoundry.sipxconfig.feature.LocationFeature;

public interface SbcManager {
    public static final LocationFeature FEATURE = new LocationFeature("borderController");
    public static final String CONTEXT_BEAN_NAME = "sbcManager";

    /**
     * returns default SBC if one if defined, otherwise null
     */
    DefaultSbc getDefaultSbc();

    /**
     * Creates default SBC if one is not found
     */
    DefaultSbc loadDefaultSbc();

    List<AuxSbc> loadAuxSbcs();

    void saveSbc(Sbc sbc);

    AuxSbc loadSbc(Integer sbcId);

    void deleteSbc(Sbc sbc);

    void removeSbcs(Collection<Integer> sbcIds);

    SbcRoutes getRoutes();

    void clear();
}
