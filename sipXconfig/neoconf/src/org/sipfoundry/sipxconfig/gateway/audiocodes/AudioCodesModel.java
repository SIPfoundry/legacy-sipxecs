/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import java.util.Set;

import org.sipfoundry.sipxconfig.gateway.GatewayModel;

public class AudioCodesModel extends GatewayModel {
    private String m_profileTemplate;
    private boolean m_fxo;
    private boolean m_fxs;
    private boolean m_digital;

    public void setProfileTemplate(String profileTemplate) {
        m_profileTemplate = profileTemplate;
    }

    public String getProfileTemplate() {
        return m_profileTemplate;
    }

    public void setFxo(boolean fxo) {
        m_fxo = fxo;
    }

    public void setFxs(boolean fxs) {
        m_fxs = fxs;
    }

    public void setDigital(boolean digital) {
        m_digital = digital;
    }

    public boolean isFxo() {
        return m_fxo;
    }

    public boolean isFxs() {
        return m_fxs;
    }

    public boolean isDigital() {
        return m_digital;
    }

    public Set getDefinitions() {
        Set definitions = super.getDefinitions();
        if (m_fxo) {
            definitions.add("fxo");
        }
        if (m_fxs) {
            definitions.add("fxs");
        }
        if (m_digital) {
            definitions.add("digital");
        }
        return definitions;
    }
}
