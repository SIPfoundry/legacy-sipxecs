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
    private boolean m_fxo;
    private boolean m_fxs;
    private boolean m_digital;
    private boolean m_bri;
    private String m_configDirectory;

    public void setFxo(boolean fxo) {
        m_fxo = fxo;
    }

    public void setFxs(boolean fxs) {
        m_fxs = fxs;
    }

    public void setDigital(boolean digital) {
        m_digital = digital;
    }

    public void setBri(boolean bri) {
        m_bri = bri;
    }

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
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

    public boolean isBri() {
        return m_bri;
    }

    public String getConfigDirectory() {
        return m_configDirectory;
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
        if (m_bri) {
            definitions.add("bri");
        }
        return definitions;
    }

}
