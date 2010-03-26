/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.setting;

import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public abstract class GroupTable extends BaseComponent {

    @Asset("/images/group.png")
    public abstract IAsset getGroupIcon();

    public abstract SettingDao getSettingContext();

    public abstract List getGroups();

    public abstract SelectMap getSelections();

    public void deleteGroup() {
        SettingDao context = getSettingContext();
        if (context.deleteGroups(getSelections().getAllSelected())) {
            IValidationDelegate validator = TapestryUtils.getValidator(getPage());
            validator.record(new ValidatorException(getMessages().getMessage("msg.error.removeAdminGroup")));
        }
    }

    public void moveUp() {
        moveGroups(-1);
    }

    public void moveDown() {
        moveGroups(1);
    }

    void moveGroups(int step) {
        SettingDao context = getSettingContext();
        context.moveGroups(getGroups(), getSelections().getAllSelected(), step);
    }
}
