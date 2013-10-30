/**
 *
 *
 * Copyright (c) 2011 / 2012 eZuce, Inc. All rights reserved.
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

package org.sipfoundry.sipxconfig.site.moh;

import java.util.Collection;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.callback.ICallback;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

public abstract class AbstractMusicOnHoldComponent extends BaseComponent {

    @Parameter
    public abstract ICallback getCallback();

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectObject(value = "spring:phoneContext")
    public abstract PhoneContext getPhoneContext();

    @InjectObject(value = "spring:eagerPhoneProfileManager")
    public abstract ProfileManager getEagerProfileManager();

    @InjectObject(value = "spring:phoneProfileManager")
    public abstract ProfileManager getLazyProfileManager();

    @InjectObject(value = "spring:configManager")
    public abstract ConfigManager getConfigManager();

    @Persist(value = "client")
    public abstract String getAsset();

    public abstract String getAudioDirectory();

    public abstract void save();

    private Collection<Integer> getPhoneIdsForUser(User user) {
        Collection<Phone> phones = getPhoneContext().getPhonesByUserId(user.getId());
        return DataCollectionUtil.extractPrimaryKeys(phones);
    }
}

