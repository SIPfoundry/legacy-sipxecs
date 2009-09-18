/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.dialplan.config.FullTransform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.permission.PermissionName;

public class VoicemailRedirectRule extends DialingRule {

    @Override
    public String[] getPatterns() {
        return new String[] {
            "."
        };
    }

    /**
     * This rule is always enabled as it is a system-generated rule
     */
    @Override
    public boolean isEnabled() {
        return true;
    }

    @Override
    public Transform[] getTransforms() {
        Transform[] transforms = new Transform[1];
        FullTransform fullTransform = new FullTransform();
        fullTransform.setUser("~~vm~{user}");
        fullTransform.setFieldParams("q=0.1");
        transforms[0] = fullTransform;
        return transforms;
    }

    @Override
    public DialingRuleType getType() {
        return DialingRuleType.MAPPING_RULE;
    }

    @Override
    public CallTag getCallTag() {
        return CallTag.VMR;
    }

    public boolean isInternal() {
        return true;
    }

    /**
     * Need "permission" elements in mappingrules.xml
     */
    @Override
    public boolean isTargetPermission() {
        return true;
    }

    public boolean isGatewayAware() {
        return false;
    }

    @Override
    public List<String> getPermissionNames() {
        return Collections.singletonList(PermissionName.VOICEMAIL.getName());
    }

    @Override
    public String getDescription() {
        return "Voicemail redirect dialing rule";
    }
}
