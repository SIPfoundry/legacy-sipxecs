/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.moh;

import java.util.Collection;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.callback.ICallback;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.moh.MusicOnHoldManager;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.Setting;

@ComponentClass
public abstract class MusicOnHoldComponent extends BaseComponent {
    @Parameter(required = true)
    public abstract User getUser();

    @Parameter
    public abstract ICallback getCallback();

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectObject(value = "spring:musicOnHoldManager")
    public abstract MusicOnHoldManager getMusicOnHoldManager();

    @InjectObject(value = "spring:phoneContext")
    public abstract PhoneContext getPhoneContext();

    @InjectObject(value = "spring:eagerPhoneProfileManager")
    public abstract ProfileManager getEagerProfileManager();

    @InjectObject(value = "spring:phoneProfileManager")
    public abstract ProfileManager getLazyProfileManager();

    @Persist
    public abstract String getAsset();

    public String getUserAudioDirectory() {
        return getMusicOnHoldManager().getUserAudioDirectory(getUser()).toString();
    }

    public Setting getMohSettings() {
        return getUser().getSettings().getSetting(User.MOH_SETTING);
    }

    public void save() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        getCoreContext().saveUser(getUser());
        getLazyProfileManager().generateProfiles(getPhoneIdsForUser(getUser()), false, null);
    }

    public void onUpdatePhones() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        getCoreContext().saveUser(getUser());
        getEagerProfileManager().generateProfiles(getPhoneIdsForUser(getUser()), true, null);
    }

    private Collection<Integer> getPhoneIdsForUser(User user) {
        Collection<Phone> phones = getPhoneContext().getPhonesByUserId(user.getId());
        return DataCollectionUtil.extractPrimaryKeys(phones);
    }
}
