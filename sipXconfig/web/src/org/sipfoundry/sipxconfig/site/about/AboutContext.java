/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.about;

import java.io.StringWriter;

import org.apache.tapestry.IPage;
import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;

public class AboutContext implements ApplicationContextAware {
    private VelocityEngine m_velocityEngine;
    private String m_aboutPluginBeanId;
    private AboutBean m_aboutBean;
    private IPage m_aboutPage;
    private ApplicationContext m_applicationContext;
    private String m_configurationFile;

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }

    public String getContent() {
        try {
            VelocityContext context = new VelocityContext();
            context.put("dollar", "$");
            context.put("defaultAbout", m_aboutBean);
            if (m_aboutPluginBeanId != null) {
                Object aboutObject = m_applicationContext.getBean(m_aboutPluginBeanId);
                context.put("about", aboutObject);
            }
            StringWriter output = new StringWriter();
            m_velocityEngine.mergeTemplate(m_configurationFile, context, output);
            output.flush();
            return output.toString();
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public void setAboutPluginBeanId(String aboutPluginBeanId) {
        m_aboutPluginBeanId = aboutPluginBeanId;
    }

    public IPage getAboutPage() {
        return m_aboutPage;
    }

    public void setAboutPage(IPage aboutPage) {
        m_aboutPage = aboutPage;
        m_aboutBean.setAboutPage(aboutPage);
    }

    public void setAboutBean(AboutBean aboutBean) {
        m_aboutBean = aboutBean;
    }

    public void setApplicationContext(ApplicationContext applicationContext) {
        m_applicationContext = applicationContext;
    }

    public void setConfigurationFile(String configurationFile) {
        m_configurationFile = configurationFile;
    }
}
