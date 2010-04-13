/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.gateway.Gateway;

public interface IDialingRule {
    public abstract String getDescription();

    public abstract void setDescription(String description);

    public abstract boolean isEnabled();

    public abstract void setEnabled(boolean enabled);

    public abstract String getName();

    public abstract void setName(String name);

    public abstract CallTag getCallTag();

    public abstract List<Gateway> getEnabledGateways();

    public abstract String[] getPatterns();

    public abstract Transform[] getTransforms();

    public Map<String, List<Transform>> getSiteTransforms();

    public abstract List<String> getPermissionNames();

    public abstract Integer getId();

    /**
     * Returns the xml tag to be used for this rule type in authrules.xml. Unless over-written for
     * a specific rule type, returns empty string and is not added to authrules.xml.
     */
    public abstract String getRuleType();

    public abstract boolean isInternal();

    /**
     * Most rules require configuring the proxy to check authorization for the calls using the
     * rules. By returning false here rule can be excluded from usual authorization mechanism. All
     * calls made using this rule will be allowed.
     */
    public abstract boolean isAuthorizationChecked();

    /**
     * Some rules are "source" permission rules - need "permission" elements in mappingrules.xml.
     */
    public abstract boolean isTargetPermission();

    /**
     * Whether or not the dialing rule can be used with a gateway.
     */
    public abstract boolean isGatewayAware();

    public abstract String[] getTransformedPatterns(Gateway gateway);

    /**
     * List of host patterns for this rule, if empty rule will be appended to default host match
     *
     * @return ip addresses, host names, or variables
     */
    public abstract String[] getHostPatterns();

    /**
     * Determines if the affected rule is enablable or not
     *
     * @return
     */
    boolean isEnablable();
}
