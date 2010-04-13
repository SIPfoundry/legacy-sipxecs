/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.sipxconfig.permission.Permission;

/**
 * SiteToSiteDialingRule
 **/
public class SiteToSiteDialingRule extends CustomDialingRule {

    @Override
    protected Object clone() throws CloneNotSupportedException {
        SiteToSiteDialingRule clone = (SiteToSiteDialingRule) super.clone();
        clone.setDialPatterns(new ArrayList(getDialPatterns()));
        return clone;
    }

    @Override
    public DialingRuleType getType() {
        return DialingRuleType.SITE_TO_SITE;
    }

    @Override
    public boolean isAuthorizationChecked() {
        return false;
    }

    @Override
    public void setPermissions(List<Permission> permissions) {
        // Do nothing. There is NO required permissions for SiteToSiteDialingRule.
        // Thus, leave m_permissionNames unset.
    }

    @Override
    public void setPermissionNames(List<String> permissionNames) {
        // Do nothing. There is NO required permissions for SiteToSiteDialingRule.
        // Thus, leave m_permissionNames unset.
    }
}
