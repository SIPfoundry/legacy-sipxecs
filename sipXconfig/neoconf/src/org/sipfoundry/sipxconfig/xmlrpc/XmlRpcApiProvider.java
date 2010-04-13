/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.xmlrpc;

import org.springframework.beans.factory.InitializingBean;

public class XmlRpcApiProvider<T> implements ApiProvider<T>, InitializingBean {

    private String m_methodNamePrefix;
    private boolean m_secure;
    private Class m_serviceInterface;
    private XmlRpcMarshaller m_marshaller;
    private long m_timeout;

    public T getApi(String serviceUrl) {
        XmlRpcProxyFactoryBean factory = new XmlRpcProxyFactoryBean();
        factory.setSecure(m_secure);
        factory.setMarshaller(m_marshaller);
        if (m_methodNamePrefix != null) {
            factory.setMethodNamePrefix(m_methodNamePrefix);
        }
        factory.setServiceInterface(m_serviceInterface);
        factory.setServiceUrl(serviceUrl);
        if (m_timeout > 0) {
            factory.setTimeout(m_timeout);
        }
        factory.afterPropertiesSet();
        return (T) factory.getObject();
    }

    public void setMethodNamePrefix(String methodNamePrefix) {
        m_methodNamePrefix = methodNamePrefix;
    }

    public void setSecure(boolean secure) {
        m_secure = secure;
    }

    public void setMarshaller(XmlRpcMarshaller marshaller) {
        m_marshaller = marshaller;
    }

    public void setServiceInterface(Class serviceInterface) {
        m_serviceInterface = serviceInterface;
    }

    public void setTimeout(long timeout) {
        m_timeout = timeout;
    }

    public void afterPropertiesSet() throws Exception {
        if (m_marshaller != null && m_methodNamePrefix != null) {
            throw new IllegalArgumentException("Specify only one: marshaller or methodNamePrefix");
        }
    }
}
