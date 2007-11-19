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
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;

public class SbcDeviceManagerImpl extends SipxHibernateDaoSupport<SbcDevice> implements
        SbcDeviceManager, BeanFactoryAware {

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
        saveBeanWithSettings(sbc);
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }
}
