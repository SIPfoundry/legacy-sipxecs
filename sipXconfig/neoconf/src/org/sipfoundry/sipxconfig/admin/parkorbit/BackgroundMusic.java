/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.parkorbit;

import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.Setting;

public class BackgroundMusic extends BeanWithGroups {
    private static final String DEFAULT = "default.wav";

    private String m_music = DEFAULT;

    private boolean m_enabled;

    public String getMusic() {
        return m_music;
    }

    public void setMusic(String music) {
        m_music = music;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    @Override
    protected Setting loadSettings() {
        // no settings for Background music
        return null;
    }
}
