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

import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.UrlTransform;

/**
 * Generic mapping rule used to forward SIP messages to various components of SIPx system
 * (Resource List Server, Park Server etc.)
 * 
 * It is always internal (added to mapping rules) and it usually has a the following form:
 * 
 * <code>
 * <userMatch>
 *   <userPattern>user part of the SIP URL</userPattern>
 *   <permissionMatch>
 *     <transform>
 *       <url>url of the SIPx server</url>
 *     </transform>
 *   </permissionMatch>
 * </userMatch>
 * </code>
 */
public abstract class InternalForwardRule extends DialingRule {

    private String m_url;
    private DialPattern m_dialPattern;

    public InternalForwardRule(DialPattern dialPattern, String url) {
        m_dialPattern = dialPattern;
        m_url = url;
        setEnabled(true);
    }

    public InternalForwardRule(String userPart, String url) {
        this(new DialPattern(userPart, 0), url);
    }

    public String[] getPatterns() {
        return new String[] {
            m_dialPattern.calculatePattern()
        };
    }

    public Transform[] getTransforms() {
        UrlTransform transform = new UrlTransform();
        transform.setUrl(m_url);
        return new Transform[] {
            transform
        };
    }

    public DialingRuleType getType() {
        return null;
    }

    public boolean isInternal() {
        return true;
    }
}
