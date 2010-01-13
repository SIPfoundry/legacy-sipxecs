/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.site.setting.GroupSettings;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class UserGroupExtContactSettings extends BaseComponent {

    private static final String SETTING_PATH_EXTCONTACT_ENABLED = "extcontact/enabled";
    private static final String SETTING_PATH_EXTCONTACT_PREFIX = "extcontact/prefix";

    private static final Log LOG = LogFactory.getLog(UserGroupExtContactSettings.class);

    @Parameter(required = true)
    public abstract Setting getSettings();

    @Parameter(required = true)
    public abstract Group getGroup();

    @Parameter(required = true)
    public abstract IValidationDelegate getValidator();

    public Boolean isExtContactEnabled() {
        return Boolean.valueOf(getSettings().getSetting(SETTING_PATH_EXTCONTACT_ENABLED).getValue());
    }

    public void setExtContactEnabled(Boolean enabled) {
        getSettings().getSetting(SETTING_PATH_EXTCONTACT_ENABLED).setTypedValue(enabled.toString());
    }

    public String getExtContactPrefix() {
        return getSettings().getSetting(SETTING_PATH_EXTCONTACT_PREFIX).getValue();
    }

    public void setExtContactPrefix(String prefix) {
        getSettings().getSetting(SETTING_PATH_EXTCONTACT_PREFIX).setValue(prefix);
    }

    public boolean validateExtContactSettings() {
        IValidationDelegate validator = TapestryUtils.getValidator(getPage());
        boolean valid = true;
        if (isExtContactEnabled()) {
            // Require that a prefix is selected
            if (getExtContactPrefix() == null) {
                validator.record(new ValidatorException(getMessages().getMessage("message.prefixRequired")));
                valid = false;
            }
        }

        return valid;
    }


    public void ok(IRequestCycle cycle) {
        if (validateExtContactSettings()) {
            GroupSettings groupPage = (GroupSettings) getPage();
            groupPage.ok(cycle);
        }
    }

    public void apply() {
        if (validateExtContactSettings()) {
            GroupSettings groupPage = (GroupSettings) getPage();
            groupPage.apply();
        }
    }

    public void cancel(IRequestCycle cycle) {
        GroupSettings groupPage = (GroupSettings) getPage();
        groupPage.cancel(cycle);
    }
}
