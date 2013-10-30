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

import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.commons.util.AudioUtil;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.moh.MusicOnHoldManager;
import org.sipfoundry.sipxconfig.setting.Group;

@ComponentClass
public abstract class GroupMusicOnHoldComponent extends AbstractMusicOnHoldComponent {

    @InjectObject(value = "spring:groupMusicOnHoldManager")
    public abstract MusicOnHoldManager getMusicOnHoldManager();

    @Parameter(required = true)
    public abstract Group getGroup();

    public String getAudioDirectory() {
        String audioDirectoryName = AudioUtil.getGroupMoHFolderName(getGroup().getName());
        return getMusicOnHoldManager().getAudioDirectory(audioDirectoryName).toString();
    }

    public void save() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        getCoreContext().saveGroup(getGroup());
        getConfigManager().configureEverywhere(MusicOnHoldManager.FEATURE);
    }

}
