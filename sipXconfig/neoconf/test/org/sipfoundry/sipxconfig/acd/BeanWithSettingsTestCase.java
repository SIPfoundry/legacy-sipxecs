/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.acd;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;
import org.sipfoundry.sipxconfig.setting.ModelFilesContextImpl;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;

public abstract class BeanWithSettingsTestCase extends TestCase {

    private ModelFilesContextImpl m_modelFilesContext;

    protected void setUp() throws Exception {
        String sysdir = TestHelper.getSysDirProperties().getProperty("sysdir.etc");
        m_modelFilesContext = new ModelFilesContextImpl();
        m_modelFilesContext.setConfigDirectory(sysdir);
        m_modelFilesContext.setModelBuilder(new XmlModelBuilder(sysdir));
    }

    protected ModelFilesContext getModelFilesContext() {
        return m_modelFilesContext;
    }

    protected void initializeBeanWithSettings(BeanWithSettings beanWithSettings) {
        beanWithSettings.setModelFilesContext(m_modelFilesContext);
    }
}
