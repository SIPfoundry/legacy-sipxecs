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

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.hibernate.Criteria;
import org.hibernate.Session;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.bridge.BridgeSbc;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.DaoEventPublisher;
import org.sipfoundry.sipxconfig.common.event.SbcDeviceDeleteListener;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.orm.hibernate3.HibernateCallback;

public class SbcDeviceManagerImpl extends SipxHibernateDaoSupport<SbcDevice> implements
        SbcDeviceManager, BeanFactoryAware {

    private static final String SBC_ID = "sbcId";

    private static final String SBC_NAME = "sbcName";

    private BeanFactory m_beanFactory;

    private DaoEventPublisher m_daoEventPublisher;

    private String m_localIpAddress;

    public void setDaoEventPublisher(DaoEventPublisher daoEventPublisher) {
        m_daoEventPublisher = daoEventPublisher;
    }

    public void clear() {
        Collection<SbcDevice> sbcs = getSbcDevices();
        for (SbcDevice sbcDevice : sbcs) {
            deleteSbcDevice(sbcDevice.getId());
        }
    }

    public void deleteSbcDevice(Integer id) {
        SbcDevice sbcDevice = getSbcDevice(id);
        m_daoEventPublisher.publishDelete(sbcDevice);
        getHibernateTemplate().delete(sbcDevice);
    }

    public void deleteSbcDevices(Collection<Integer> ids) {
        for (Integer id : ids) {
            SbcDevice sbcDevice = getSbcDevice(id);
            m_daoEventPublisher.publishDelete(sbcDevice);
        }
        removeAll(SbcDevice.class, ids);
    }

    public SbcDeviceDeleteListener createSbcDeviceDeleteListener() {
        return new OnSbcDeviceDelete();
    }

    public Collection<Integer> getAllSbcDeviceIds() {
        return getHibernateTemplate().findByNamedQuery("sbcIds");
    }

    public SbcDevice getSbcDevice(Integer id) {
        return load(SbcDevice.class, id);
    }

    public BridgeSbc getBridgeSbc() {
        SbcDevice sbcDevice = DaoUtils.requireOneOrZero(getSbcDeviceByType(BridgeSbc.class), "sbc bridge");
        return (BridgeSbc) sbcDevice;
    }

    private <T> List<T> getSbcDeviceByType(final Class<T> type) {
        HibernateCallback callback = new HibernateCallback() {
            public Object doInHibernate(Session session) {
                Criteria criteria = session.createCriteria(type);
                return criteria.list();
            }
        };
        return getHibernateTemplate().executeFind(callback);
    }

    private List<SbcDevice> getSbcDevicesByDescriptor(SbcDescriptor descriptor) {
        List<SbcDevice> sbcs = getSbcDevices();
        List<SbcDevice> list = new ArrayList<SbcDevice>();
        for (SbcDevice sbc : sbcs) {
            if (sbc.getModelId().equals(descriptor.getModelId())) {
                list.add(sbc);
            }
        }
        return list;
    }

    public List<SbcDevice> getSbcDevices() {
        return getHibernateTemplate().loadAll(SbcDevice.class);
    }

    public void checkForNewSbcDeviceCreation(SbcDescriptor descriptor) {
        int maxAllowed = descriptor.getMaxAllowed();
        if (descriptor.getMaxAllowed() > -1) {
            int size = getSbcDevicesByDescriptor(descriptor).size();
            if (size >= maxAllowed) {
                throw new UserException("sbc.creation.error", new Object[] {size, descriptor.getLabel()});
            }
        }
    }

    public SbcDevice newSbcDevice(SbcDescriptor descriptor) {
        String beanId = descriptor.getBeanId();
        SbcDevice newSbc = (SbcDevice) m_beanFactory.getBean(beanId, SbcDevice.class);
        newSbc.setModel(descriptor);
        newSbc.setAddress(m_localIpAddress);
        return newSbc;
    }

    public void storeSbcDevice(SbcDevice sbc) {
        if (sbc.isNew()) {
            checkForNewSbcDeviceCreation(sbc.getModel());
            checkForDuplicateNames(sbc);
        } else {
            // if the sbc name was changed
            if (isNameChanged(sbc)) {
                checkForDuplicateNames(sbc);
            }
        }
        saveBeanWithSettings(sbc);
    }

    private void checkForDuplicateNames(SbcDevice sbc) {
        if (isNameInUse(sbc)) {
            throw new UserException("error.duplicateSbcName");
        }
    }

    private boolean isNameInUse(SbcDevice sbc) {
        List count = getHibernateTemplate().findByNamedQueryAndNamedParam(
                "anotherSbcWithSameName", new String[] {
                    SBC_NAME
                }, new Object[] {
                    sbc.getName()
                });

        return DataAccessUtils.intResult(count) > 0;
    }

    private boolean isNameChanged(SbcDevice sbc) {
        List count = getHibernateTemplate().findByNamedQueryAndNamedParam("countSbcWithSameName",
                new String[] {
                    SBC_ID, SBC_NAME
                }, new Object[] {
                    sbc.getId(), sbc.getName()
                });

        return DataAccessUtils.intResult(count) == 0;
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    private List<Sbc> getSbcsForSbcDeviceId(Integer sbcDeviceId) {
        return getHibernateTemplate().findByNamedQueryAndNamedParam("sbcsForSbcDeviceId", SBC_ID,
                sbcDeviceId);
    }

    private class OnSbcDeviceDelete extends SbcDeviceDeleteListener {
        @Override
        protected void onSbcDeviceDelete(SbcDevice sbcDevice) {
            // get all SBCs associated
            List<Sbc> sbcs = getSbcsForSbcDeviceId(sbcDevice.getId());
            for (Sbc sbc : sbcs) {
                if (sbc.onDeleteSbcDevice()) {
                    getHibernateTemplate().delete(sbc);
                } else {
                    getHibernateTemplate().saveOrUpdate(sbc);
                }
            }
        }
    }

    public void setLocalIpAddress(String localIpAddress) {
        m_localIpAddress = localIpAddress;
    }

    public String getLocalIpAddress() {
        return m_localIpAddress;
    }
}
