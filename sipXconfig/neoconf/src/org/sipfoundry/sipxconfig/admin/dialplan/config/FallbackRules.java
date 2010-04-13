/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;

/**
 * Special type of fallbackrules document with a single host match matching standard sipX hosts
 *
 */
public class FallbackRules extends MappingRules {
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/fallback-00-00";

    public FallbackRules() {
        super(NAMESPACE);
    }

    @Override
    public void generate(IDialingRule rule) {
        if (!rule.isInternal()) {
            generateRule(rule);
        }
    }

    @Override
    protected void addTransforms(IDialingRule rule, Element userMatch) {
        Map<String, List<Transform>> siteMap = rule.getSiteTransforms();
        Set<String> sites = siteMap.keySet();
        for (String site : sites) {
            if (StringUtils.isEmpty(site)) {
                // empty site means shared transforms
                continue;
            }
            Element callerLocationMatch = userMatch.addElement("callerLocationMatch");
            Element callerLocation = callerLocationMatch.addElement("callerLocation");
            callerLocation.setText(site);
            List<Transform> transforms = siteMap.get(site);
            for (Transform transform : transforms) {
                transform.addToParent(callerLocationMatch);
            }
        }
        List<Transform> sharedList = siteMap.get(StringUtils.EMPTY);
        if (sharedList != null) {
            for (Transform transform : sharedList) {
                transform.addToParent(userMatch);
            }
        }
    }
}
