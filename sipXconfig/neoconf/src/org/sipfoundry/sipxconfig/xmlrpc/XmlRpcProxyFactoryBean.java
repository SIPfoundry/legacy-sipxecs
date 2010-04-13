/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.xmlrpc;

import org.springframework.aop.framework.ProxyFactory;
import org.springframework.beans.factory.FactoryBean;

public class XmlRpcProxyFactoryBean extends XmlRpcClientInterceptor implements FactoryBean {

    private Object m_serviceProxy;

    public void afterPropertiesSet() {
        super.afterPropertiesSet();
        m_serviceProxy = ProxyFactory.getProxy(getServiceInterface(), this);
    }

    public Object getObject() {
        return m_serviceProxy;
    }

    public Class getObjectType() {
        return m_serviceProxy != null ? m_serviceProxy.getClass() : getServiceInterface();
    }

    public boolean isSingleton() {
        return true;
    }
}
