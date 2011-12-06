/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.dialplan;

import org.sipfoundry.sipxconfig.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.dialplan.config.UrlTransform;

/**
 * Generic mapping rule used to forward SIP messages to various components of SIPx system
 * (Resource List Server, Park Server etc.)
 *
 * It is always internal (added to mapping rules) and it usually has the following form:
 *
 * <code>
 * &lt;userMatch&gt;
 *   &lt;userPattern&gt;user part of the SIP URL&lt;/userPattern&gt;
 *   &lt;permissionMatch&gt;
 *     &lt;transform&gt;
 *       &lt;url&gt;url of the SIPx server&lt;/url&gt;
 *     &lt;/transform&gt;
 *   &lt;/permissionMatch&gt;
 * &lt;/userMatch&gt;
 * </code>
 */
public abstract class InternalForwardRule extends DialingRule {

    private final DialPattern m_dialPattern;
    private final Transform m_transform;

    public InternalForwardRule(DialPattern dialPattern, String url) {
        m_dialPattern = dialPattern;
        UrlTransform transform = new UrlTransform();
        transform.setUrl(url);
        m_transform = transform;
        setEnabled(true);
    }

    public InternalForwardRule(DialPattern dialPattern, Transform transform) {
        m_dialPattern = dialPattern;
        m_transform = transform;
        setEnabled(true);
    }

    public InternalForwardRule(String userPart, String url) {
        this(new DialPattern(userPart, 0), url);
    }

    @Override
    public String[] getPatterns() {
        return new String[] {
            m_dialPattern.calculatePattern()
        };
    }

    @Override
    public Transform[] getTransforms() {
        return new Transform[] {
            m_transform
        };
    }

    @Override
    public DialingRuleType getType() {
        return null;
    }

    public boolean isInternal() {
        return true;
    }

    public boolean isGatewayAware() {
        return false;
    }
}
