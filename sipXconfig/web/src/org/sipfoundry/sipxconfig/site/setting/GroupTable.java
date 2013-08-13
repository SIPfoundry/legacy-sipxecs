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

import static org.apache.commons.lang.StringUtils.equalsIgnoreCase;

import java.util.Collection;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public abstract class GroupTable extends BaseComponent {

    @Asset("/images/group.png")
    public abstract IAsset getGroupIcon();

    @InjectObject("spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectObject("spring:phoneContext")
    public abstract PhoneContext getPhoneContext();

    @InjectObject("spring:settingDao")
    public abstract SettingDao getSettingContext();

    @Parameter(required = false)
    public abstract String getResource();

    public abstract List getGroups();

    public abstract SelectMap getSelections();

    public void deleteGroup() {
        SettingDao context = getSettingContext();
        Collection selected = getSelections().getAllSelected();
        if (selected.isEmpty()) {
            throw new EmptySelectionException();
        } else {
            boolean result = false;
            if (equalsIgnoreCase(getResource(), CoreContext.USER_GROUP_RESOURCE_ID)) {
                result = getCoreContext().deleteGroups(selected);
            } else if (equalsIgnoreCase(getResource(), PhoneContext.GROUP_RESOURCE_ID)) {
                result = getPhoneContext().deleteGroups(selected);
            } else {
                result = context.deleteGroups(selected);
            }
            if (result) {
                IValidationDelegate validator = TapestryUtils.getValidator(getPage());
                validator.record(new ValidatorException(getMessages().getMessage("msg.error.removeAdminGroup")));
            }
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

    private static class EmptySelectionException extends UserException {
        public EmptySelectionException() {
            super("&msg.error.emptySelection");
        }
    }
}
