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

import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.dao.support.DataAccessUtils;

public class SbcDeviceManagerImpl extends SipxHibernateDaoSupport<SbcDevice> implements
        SbcDeviceManager, BeanFactoryAware {

    private static final String SBC_ID = "sbcId";

    private static final String SBC_NAME = "sbcName";

    private BeanFactory m_beanFactory;

    public void clear() {
        Collection<SbcDevice> sbcs = getSbcDevices();
        getHibernateTemplate().deleteAll(sbcs);
    }

    public void deleteSbcDevice(Integer id) {
        SbcDevice sbcDevice = getSbcDevice(id);
        getHibernateTemplate().delete(sbcDevice);
    }

    public void deleteSbcDevices(Collection<Integer> ids) {
        removeAll(SbcDevice.class, ids);
    }

    public Collection<Integer> getAllSbcDeviceIds() {
        return getHibernateTemplate().findByNamedQuery("sbcIds");        
    }

    public SbcDevice getSbcDevice(Integer id) {
        return load(SbcDevice.class, id);
    }

    public List<SbcDevice> getSbcDevices() {
        return getHibernateTemplate().loadAll(SbcDevice.class);
    }

    public SbcDevice newSbcDevice(SbcDescriptor descriptor) {
        String beanId = descriptor.getBeanId();
        SbcDevice newSbc = (SbcDevice) m_beanFactory.getBean(beanId, SbcDevice.class);
        newSbc.setModel(descriptor);
        return newSbc;
    }

    public void storeSbcDevice(SbcDevice sbc) {
        if (sbc.isNew()) {
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
}
