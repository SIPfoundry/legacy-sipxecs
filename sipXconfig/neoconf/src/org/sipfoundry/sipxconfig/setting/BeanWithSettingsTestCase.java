/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.setting;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.test.TestHelper;

public abstract class BeanWithSettingsTestCase extends TestCase {

    private ModelFilesContextImpl m_modelFilesContext;

    protected void setUp() throws Exception {
        String sysdir = TestHelper.getSystemEtcDir();
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
