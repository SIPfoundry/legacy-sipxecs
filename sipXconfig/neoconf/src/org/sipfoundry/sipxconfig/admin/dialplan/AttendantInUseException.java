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

import java.text.MessageFormat;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.UserException;

public class AttendantInUseException extends UserException {
    // TODO: localize messages
    static final String OPERATOR_DELETE = "You cannot delete the operator attendant";
    static final String IN_USE = "The attendant cannot be deleted. It is used by the following "
            + "dialing rule(s): {0}.";

    /**
     * if null this is "operator delete" exception - may be it should be a different class...
     * otherwise it keeps the list of rules to be deleted
     */
    private Collection m_rules;

    /** Thrown when user tries to delete operator */
    public AttendantInUseException() {
        m_rules = null;
    }

    public AttendantInUseException(Collection rules) {
        m_rules = rules;
    }

    public String getMessage() {
        if (null == m_rules) {
            return OPERATOR_DELETE;
        }
        List names = new ArrayList(m_rules.size());
        for (Iterator i = m_rules.iterator(); i.hasNext();) {
            DialingRule rule = (DialingRule) i.next();
            names.add(rule.getName());
        }
        String ruleNames = StringUtils.join(names.iterator(), ", ");
        return MessageFormat.format(IN_USE, new Object[] {
            ruleNames
        });
    }
}
