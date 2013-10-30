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

import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.commons.util.AudioUtil;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.moh.MusicOnHoldManager;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;

@ComponentClass
public abstract class MusicOnHoldComponent extends AbstractMusicOnHoldComponent {

    @InjectObject(value = "spring:musicOnHoldManager")
    public abstract MusicOnHoldManager getMusicOnHoldManager();

    @Parameter(required = true)
    public abstract User getUser();

    public String getAudioDirectory() {
        String audioDirectoryName = getUser().getName();
        if (getIsGroupAudioSource()) {
            Group highestWeightGroup = Group.selectGroupWithHighestWeight(getUser().getGroupsAsList());
            if (highestWeightGroup != null) {
                audioDirectoryName = AudioUtil.getGroupMoHFolderName(highestWeightGroup
                        .getName());
            }
        }
        return getMusicOnHoldManager().getAudioDirectory(audioDirectoryName).toString();
    }

    public Setting getMohSettings() {
        return getUser().getSettings().getSetting(User.MOH_SETTING);
    }

    public Setting getMohAudioSettings() {
        return getUser().getSettings().getSetting(User.MOH_AUDIO_SOURCE_SETTING);
    }

    public void save() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        getCoreContext().saveUser(getUser());
        getConfigManager().configureEverywhere(MusicOnHoldManager.FEATURE);
    }

    public boolean getIsGroupAudioSource() {
        String mohAudioSettings = getMohAudioSettings().getValue();
        if (mohAudioSettings != null) {
            if (User.MohAudioSource.parseSetting(mohAudioSettings) == User.MohAudioSource.GROUP_FILES_SRC) {
                return true;
            }
        }
        return false;
    }

    /**
     * if user is part of a group, check if it has MoH permission
     */
    public boolean getHasConfigureMoHPermission() {

        if (getUser().getGroupsAsList().size() > 0) {
            return getUser().hasPermission(PermissionName.GROUP_MUSIC_ON_HOLD);
        }

        return true;
    }
}
