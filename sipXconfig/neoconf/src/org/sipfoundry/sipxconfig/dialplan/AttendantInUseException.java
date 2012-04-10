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

import org.sipfoundry.sipxconfig.common.UserException;

public class AttendantInUseException extends UserException {
    static final String OPERATOR_DELETE = "&error.operatorDelete";
    static final String IN_USE = "&error.attendantInUse";

    /** Thrown when user tries to delete operator */
    public AttendantInUseException() {
        super(OPERATOR_DELETE);
    }

    /**
     * List of rules to be deleted
     */
    public AttendantInUseException(Object[] rules) {
        super(IN_USE, rules);
    }
}
