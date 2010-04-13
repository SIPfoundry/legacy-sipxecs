/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.nattraversal;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.InitTaskListener;
import org.springframework.beans.factory.annotation.Required;

public class NatTraversalDefaultsMigrationTask extends InitTaskListener {

    private static final Log LOG = LogFactory.getLog(NatTraversalDefaultsMigrationTask.class);

    private NatTraversalManager m_natTraversalManager;

    @Override
    public void onInitTask(String task) {
        LOG.info("Starting nattraversal preferences migration");

        LOG.info("Found BehindNatSetting: " + m_natTraversalManager.getNatTraversal().isBehindnat());
        NatTraversal natTraversal = m_natTraversalManager.getNatTraversal();

        natTraversal.setBehindnat(!m_natTraversalManager.getNatTraversal().isBehindnat());
        natTraversal.setEnabled((!m_natTraversalManager.getNatTraversal().isEnabled()));

        m_natTraversalManager.store(natTraversal);
    }

    @Required
    public void setNatTraversalManager(NatTraversalManager natTraversalManager) {
        m_natTraversalManager = natTraversalManager;
    }

}
