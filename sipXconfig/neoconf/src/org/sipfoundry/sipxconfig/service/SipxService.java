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

import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class SipxService extends BeanWithSettings {

    private String m_beanId;
    private String m_modelName;
    private String m_modelDir;
    private SipxServiceConfiguration m_configuration;
    
    public String getBeanId() {
        return m_beanId;
    }
    
    public void setBeanId(String beanId) {
        m_beanId = beanId;
    }
    
    public String getModelName() {
        return m_modelName;
    }
    
    public void setModelName(String modelName) {
        m_modelName = modelName;
    }
    
    public String getModelDir() {
        return m_modelDir;
    }
    
    public void setModelDir(String modelDir) {
        m_modelDir = modelDir;
    }
    
    public void setConfiguration(SipxServiceConfiguration configuration) {
        m_configuration = configuration;
    }
    
    public SipxServiceConfiguration getConfiguration() {
        return m_configuration;
    }
    
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile(m_modelName, m_modelDir);
    }
}
