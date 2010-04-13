/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.sbc;

import java.sql.Connection;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.hibernate.classic.Session;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.bridge.BridgeSbc;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.device.BeanFactoryModelSource;

public class SbcMigrationContextImpl extends SipxHibernateDaoSupport implements SbcMigrationContext {
    public static final Log LOG = LogFactory.getLog(SbcMigrationContextImpl.class);

    private static final String SQL = "alter table sbc drop column address";

    private SbcDeviceManager m_sbcDeviceManager;

    private BeanFactoryModelSource<SbcDescriptor> m_sbcModelSource;

    private Sbc getSbc(Integer id) {
        return (Sbc) getHibernateTemplate().load(Sbc.class, id);
    }

    public void migrateSbc() {
        List sbcs = getHibernateTemplate().findByNamedQuery("allSbcs");
        for (Iterator i = sbcs.iterator(); i.hasNext();) {
            Object[] row = (Object[]) i.next();
            Integer sbcId = (Integer) row[0];
            String address = (String) row[1];
            if (sbcId == null || address == null) {
                continue;
            }
            try {
                Integer sbcDeviceId = createAssociateSbcDevice(address);
                Sbc sbc = getSbc(sbcId);
                sbc.setSbcDevice(m_sbcDeviceManager.getSbcDevice(sbcDeviceId));
                SbcDevice sbcDevice = sbc.getSbcDevice();
                if (sbcDevice instanceof BridgeSbc) {
                    ((BridgeSbc) sbcDevice).updateBridgeLocationId();
                }
                getHibernateTemplate().save(sbc);
                getHibernateTemplate().flush();
            } catch (UserException e) {
                LOG.warn("cannot migrate sbcs", e);
            }
        }

        cleanSchema();
    }

    private Integer createAssociateSbcDevice(String address) {
        SbcDescriptor sbcGenericModel = m_sbcModelSource.getModel("sbcGenericModel");
        SbcDevice sbcDevice = m_sbcDeviceManager.newSbcDevice(sbcGenericModel);
        // prevent name conflicts
        sbcDevice.setName(address + "_" + System.currentTimeMillis());
        sbcDevice.setAddress(address);
        m_sbcDeviceManager.storeSbcDevice(sbcDevice);
        getHibernateTemplate().flush();

        return sbcDevice.getId();
    }

    private void cleanSchema() {
        try {
            Session currentSession = getHibernateTemplate().getSessionFactory().getCurrentSession();
            Connection connection = currentSession.connection();
            Statement statement = connection.createStatement();
            statement.addBatch(SQL);
            statement.executeBatch();
            statement.close();
        } catch (SQLException e) {
            LOG.warn("cleaning schema", e);
        }
    }

    public void setSbcDeviceManager(SbcDeviceManager sbcDeviceManager) {
        m_sbcDeviceManager = sbcDeviceManager;
    }

    public void setSbcModelSource(BeanFactoryModelSource<SbcDescriptor> sbcModelSource) {
        m_sbcModelSource = sbcModelSource;
    }
}
