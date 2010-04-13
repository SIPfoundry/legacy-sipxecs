/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

public class ServiceManagerImpl extends SipxHibernateDaoSupport<ConfiguredService>
        implements BeanFactoryAware, ServiceManager {

    private ListableBeanFactory m_beanFactory;

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    public ConfiguredService loadService(Integer serviceId) {
        return (ConfiguredService) getHibernateTemplate().load(ConfiguredService.class, serviceId);
    }

    public void deleteServices(Collection<Integer> serviceIds) {
        removeAll(ConfiguredService.class, serviceIds);
    }

    public void deleteService(ConfiguredService service) {
        deleteBeanWithSettings(service);
    }

    public ConfiguredService newService(ServiceDescriptor descriptor) {
        ConfiguredService service = (ConfiguredService) m_beanFactory.getBean(descriptor.getBeanId());
        service.setDescriptorId(descriptor.getModelId());
        return service;
    }

    public ConfiguredService getServiceByName(String name) {
        String query = "service-by-name";
        Collection<ConfiguredService> services = getHibernateTemplate().findByNamedQueryAndNamedParam(
                query, "name", name);
        return DaoUtils.requireOneOrZero(services, query);
    }

    public void saveService(ConfiguredService service) {
        DaoUtils.checkDuplicatesByNamedQuery(getHibernateTemplate(), service, "service-ids-by-name", service.getName(),
                new DuplicateNamedServiceException(service.getName()));
        saveBeanWithSettings(service);
    }

    class DuplicateNamedServiceException extends UserException {
        DuplicateNamedServiceException(String name) {
            super(String.format("A service with name %s already exists", name));
        }
    }

    public List<ConfiguredService> getServices() {
        return getHibernateTemplate().findByNamedQuery("services");
    }

    public List<ConfiguredService> getEnabledServicesByType(ServiceDescriptor descriptor) {
        return getHibernateTemplate().findByNamedQueryAndNamedParam("enabled-services-by-id", "descriptorId",
                descriptor.getModelId());
    }

    public void clear() {
        getHibernateTemplate().deleteAll(getServices());
    }
}
