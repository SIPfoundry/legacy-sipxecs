/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 *
 * Incarnation from http://sourceforge.net/projects/diaphragma/ project
 */
package org.sipfoundry.sipxconfig.site;

import org.apache.hivemind.events.RegistryShutdownListener;
import org.apache.tapestry.web.WebContext;
import org.sipfoundry.sipxconfig.components.HivemindContext;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.context.ConfigurableApplicationContext;
import org.springframework.web.context.WebApplicationContext;

/**
 * Glue between Tapestry and Spring using Hiveminds builtin Spring integration
 */
public class SpringBeanFactoryHolderImpl extends
        org.apache.hivemind.lib.impl.SpringBeanFactoryHolderImpl implements
        RegistryShutdownListener {

    private WebContext m_context;
    private HivemindContext m_hivemindContext;

    public HivemindContext getHivemindContext() {
        return m_hivemindContext;
    }

    public void setHivemindContext(HivemindContext hivemindContext) {
        m_hivemindContext = hivemindContext;
    }

    public void setContext(WebContext context) {
        m_context = context;
    }

    public WebContext getContext() {
        return m_context;
    }

    public BeanFactory getBeanFactory() {
        if (super.getBeanFactory() == null) {
            BeanFactory factory = getWebApplicationContext(getContext());
            TapestryContext tapestry = (TapestryContext) factory.getBean(TapestryContext.CONTEXT_BEAN_NAME);
            tapestry.setHivemindContext(getHivemindContext());
            super.setBeanFactory(factory);
        }
        return super.getBeanFactory();
    }

    public static WebApplicationContext getWebApplicationContext(WebContext wc) {
        Object attr = wc
                .getAttribute(WebApplicationContext.ROOT_WEB_APPLICATION_CONTEXT_ATTRIBUTE);
        if (attr == null) {
            return null;
        }
        if (attr instanceof RuntimeException) {
            throw (RuntimeException) attr;
        }
        if (attr instanceof Error) {
            throw (Error) attr;
        }
        if (!(attr instanceof WebApplicationContext)) {
            throw new IllegalStateException(
                    "Root context attribute is not of type WebApplicationContext: " + attr);
        }
        return (WebApplicationContext) attr;
    }

    public void registryDidShutdown() {
        ((ConfigurableApplicationContext) super.getBeanFactory()).close();
    }
}
