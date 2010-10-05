/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.gateway;

import java.util.Iterator;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDescriptor;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDevice;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.device.BeanFactoryModelSource;
import org.sipfoundry.sipxconfig.setting.Storage;

public class SipTrunkMigrationContextImpl extends SipxHibernateDaoSupport implements
        SipTrunkMigrationContext {
    public static final Log LOG = LogFactory.getLog(SipTrunkMigrationContextImpl.class);

    private GatewayContext m_gatewayContext;

    private SbcDeviceManager m_sbcDeviceManager;

    private BeanFactoryModelSource<SbcDescriptor> m_sbcModelSource;

    public void migrateSipTrunk() {
        List sipTrunks = getHibernateTemplate().findByNamedQuery("sipTrunks");
        for (Iterator i = sipTrunks.iterator(); i.hasNext();) {
            Object[] row = (Object[]) i.next();
            Integer sipTrunkId = (Integer) row[0];
            String address = (String) row[1];
            if (sipTrunkId == null || address == null) {
                continue;
            }
            try {
                Integer sbcDeviceId = createAssociateSbcDevice(address);
                Gateway sipTrunk = m_gatewayContext.getGateway(sipTrunkId);

                // clean value storage
                Storage valueStorage = sipTrunk.getValueStorage();
                getHibernateTemplate().delete(valueStorage);
                sipTrunk.setValueStorage(null);

                sipTrunk.setSbcDevice(m_sbcDeviceManager.getSbcDevice(sbcDeviceId));
                sipTrunk.setOutboundAddress(sipTrunk.getSbcDevice().getAddress());
                sipTrunk.setOutboundPort(sipTrunk.getSbcDevice().getPort());
                m_gatewayContext.storeGateway(sipTrunk);
                getHibernateTemplate().flush();
            } catch (UserException e) {
                LOG.warn("cannot migrate sip trunks", e);
            }
        }
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

    public void setGatewayContext(GatewayContext gatewayContext) {
        m_gatewayContext = gatewayContext;
    }

    public void setSbcDeviceManager(SbcDeviceManager sbcDeviceManager) {
        m_sbcDeviceManager = sbcDeviceManager;
    }

    public void setSbcModelSource(BeanFactoryModelSource<SbcDescriptor> sbcModelSource) {
        m_sbcModelSource = sbcModelSource;
    }
}
