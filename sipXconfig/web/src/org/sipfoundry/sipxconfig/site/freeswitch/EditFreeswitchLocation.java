/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.site.freeswitch;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchSettings;

public abstract class EditFreeswitchLocation extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "freeswitch/EditFreeswitchLocation";

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract Integer getLocationId();

    public abstract void setLocationId(Integer locationId);

    @InjectObject("spring:freeswitchFeature")
    public abstract FreeswitchFeature getFreeswitchFeature();

    public abstract FreeswitchSettings getSettings();

    public abstract void setSettings(FreeswitchSettings settings);

    @Override
    public void pageBeginRender(PageEvent arg0) {
        if (getSettings() == null) {
            setSettings(getFreeswitchFeature().getSettings(getLocationsManager().getLocation(getLocationId())));
        }
    }

    public void apply() {
        getSettings().setLocation(getLocationsManager().getLocation(getLocationId()));
        getFreeswitchFeature().saveSettings(getSettings());
    }

    @Override
    public String getBreadCrumbTitle() {
        return null == getLocationId() ? "&crumb.edit.freeswitch"
            : getLocationsManager().getLocation(getLocationId()).getFqdn();
    }
}
