/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.snmp;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.snmp4j.smi.Counter32;
import org.snmp4j.smi.OID;
import org.snmp4j.smi.VariableBinding;
import org.snmp4j.util.TableEvent;
import org.snmp4j.util.TableListener;

/**
 * Reading SNMP info can be cumbersome, this attempts to organize the reading into a tree
 * where you can intercept data in a tree-like structure by implementing a SnmpTableWalker
 */
public class SnmpTableResultsParser implements TableListener {
    private static final Log LOG = LogFactory.getLog(SnmpTableResultsParser.class);
    private boolean m_finished;
    private OID m_previous;
    private SnmpTableWalker m_walker;

    public SnmpTableResultsParser(SnmpTableWalker walker) {
        m_walker = walker;
    }

    public void finished(TableEvent event) {
        LOG.info("Table walk completed with status " + event.getStatus() + ". Received " + event.getUserObject()
                + " rows.");
        m_finished = true;
        synchronized (event.getUserObject()) {
            event.getUserObject().notify();
        }
    }

    public boolean next(TableEvent event) {
        for (VariableBinding vb : event.getColumns()) {
            OID oid = vb.getOid();
            int depth = oid.size();
            if (m_previous != null && oid.leftMostCompare(depth - 1, m_previous) != 0) {
                m_walker.up();
            }
            m_walker.next(vb);
            m_previous = oid;
        }
        ((Counter32) event.getUserObject()).increment();
        return true;
    }

    public boolean isFinished() {
        return m_finished;
    }
}
