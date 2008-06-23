/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.nattraversal;

import org.sipfoundry.sipxconfig.common.InitTaskListener;

public class NatTraversalInit extends InitTaskListener {
    private NatTraversalManager m_natTraversalManager;

    @Override
    public void onInitTask(String task) {
        m_natTraversalManager.saveDefaultNatTraversal();
    }

    public void setNatTraversalManager(NatTraversalManager natTraversalManager) {
        m_natTraversalManager = natTraversalManager;
    }

}
