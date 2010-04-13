/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import org.sipfoundry.sipxconfig.common.UserException;

public class NameInUseException extends UserException {
    private static final String ERROR_LONG = "The name \"{1}\" is already being used by a user or service.  "
            + "Please choose another name for this {0}.";

    private static final String ERROR_SHORT = "The name \"{0}\" is already being used by a user or service.";

    public NameInUseException(String objectType, String name) {
        super(ERROR_LONG, objectType, name);
    }

    public NameInUseException(String name) {
        super(ERROR_SHORT, name);
    }
}
