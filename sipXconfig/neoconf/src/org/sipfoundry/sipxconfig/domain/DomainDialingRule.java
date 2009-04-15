/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.domain;

import java.util.Collections;
import java.util.List;
import java.util.Set;

import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleType;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;

/**
 * Generate the following into the mappingrules.xml <code>
 * <hostMatch>
 *   <hostPattern>DOMAIN ALIAS #1 HERE</hostPattern>
 *   <hostPattern>DOMAIN ALIAS #2 HERE</hostPattern>
 *   <userMatch>
 *     <userPattern>.</userPattern>
 *     <permissionMatch>
 *       <transform>
 *         <host>ACTUAL DOMAIN HERE</host>
 *       </transform>
 *     </permissionMatch>
 *   </userMatch>
 * </hostMatch>
 * </code>
 */
public class DomainDialingRule extends DialingRule {
    private static final String[] MATCH_ALL = new String[] {
        "."
    };
    private final Domain m_domain;

    DomainDialingRule(Domain domain) {
        m_domain = domain;
        setEnabled(true);
    }

    @Override
    public String[] getPatterns() {
        return MATCH_ALL;
    }

    @Override
    public Transform[] getTransforms() {
        Transform hostTransform = new Transform() {
            @Override
            protected void addChildren(Element transform) {
                Element host = transform.addElement("host");
                host.addText(m_domain.getName());
            }
        };
        return new Transform[] {
            hostTransform
        };
    }

    @Override
    public DialingRuleType getType() {
        // no type
        return null;
    }

    @Override
    public List<String> getPermissionNames() {
        return Collections.emptyList();
    }

    public boolean isInternal() {
        return true;
    }

    public boolean isGatewayAware() {
        return false;
    }

    @Override
    public String[] getHostPatterns() {
        Set<String> aliases = m_domain.getAliases();
        String[] hostPatterns = aliases.toArray(new String[aliases.size()]);
        return hostPatterns;
    }
}
