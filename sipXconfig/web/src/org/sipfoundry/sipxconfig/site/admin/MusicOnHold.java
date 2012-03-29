/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.moh.MohSettings;
import org.sipfoundry.sipxconfig.moh.MusicOnHoldManager;

public abstract class MusicOnHold extends SipxBasePage implements PageBeginRenderListener {

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject("spring:musicOnHoldManager")
    public abstract MusicOnHoldManager getMusicOnHoldManager();

    public abstract MohSettings getSettings();

    public abstract void setSettings(MohSettings settings);

    public abstract String getAsset();

    public abstract Boolean getAudioDirectoryEmpty();

    public abstract void setAudioDirectoryEmpty(Boolean isAudioDirectoryEmpty);

    public void pageBeginRender(PageEvent event_) {
        if (getSettings() == null) {
            setSettings(getMusicOnHoldManager().getSettings());
        }
        boolean managerDirectoryEmpty = getMusicOnHoldManager().isAudioDirectoryEmpty();
        if (getAudioDirectoryEmpty() == null) {
            setAudioDirectoryEmpty(managerDirectoryEmpty);
        } else if (managerDirectoryEmpty != getAudioDirectoryEmpty()) {
            setAudioDirectoryEmpty(managerDirectoryEmpty);
        }

    }

    public void saveValid() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        getMusicOnHoldManager().saveSettings(getSettings());
    }
}
