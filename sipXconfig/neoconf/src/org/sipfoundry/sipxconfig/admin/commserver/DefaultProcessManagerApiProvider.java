/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcProxyFactoryBean;

public class DefaultProcessManagerApiProvider implements ProcessManagerApiProvider {

    private String m_methodNamePrefix;
    private boolean m_secure;
    private Class m_serviceInterface;

    public ProcessManagerApi getApi(Location location) {
        String serviceUrl = location.getProcessMonitorUrl();
        return getApi(serviceUrl);
    }

    private ProcessManagerApi getApi(String serviceUrl) {
        XmlRpcProxyFactoryBean factory = new XmlRpcProxyFactoryBean();
        factory.setSecure(m_secure);
        factory.setMethodNamePrefix(m_methodNamePrefix);
        factory.setServiceInterface(m_serviceInterface);
        factory.setServiceUrl(serviceUrl);
        factory.afterPropertiesSet();
        return (ProcessManagerApi) factory.getObject();
    }

    public void setMethodNamePrefix(String methodNamePrefix) {
        m_methodNamePrefix = methodNamePrefix;
    }

    public void setSecure(boolean secure) {
        m_secure = secure;
    }

    public void setServiceInterface(Class serviceInterface) {
        m_serviceInterface = serviceInterface;
    }
}
