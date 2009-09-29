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

public interface SbcManager {
    public static final String CONTEXT_BEAN_NAME = "sbcManager";

    DefaultSbc loadDefaultSbc();

    List<AuxSbc> loadAuxSbcs();

    void saveSbc(Sbc sbc);

    AuxSbc loadSbc(Integer sbcId);

    void removeSbcs(Collection<Integer> sbcIds);

    SbcRoutes getRoutes();

    void clear();
}
