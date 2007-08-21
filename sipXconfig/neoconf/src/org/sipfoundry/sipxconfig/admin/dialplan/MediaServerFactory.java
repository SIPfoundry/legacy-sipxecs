/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.Collection;

import org.apache.commons.lang.StringUtils;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;

public class MediaServerFactory implements BeanFactoryAware {

    private BeanFactory m_beanFactory;
    private Collection<String> m_beanIds;

    public MediaServer create(String id) {
        MediaServer server = (MediaServer) m_beanFactory.getBean(id, MediaServer.class);
        server.setHostname(StringUtils.EMPTY);
        server.setServerExtension(StringUtils.EMPTY);
        return server;
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    public void setBeanIds(Collection<String> beanIds) {
        m_beanIds = beanIds;
    }

    public Collection<String> getBeanIds() {
        return m_beanIds;
    }
}
