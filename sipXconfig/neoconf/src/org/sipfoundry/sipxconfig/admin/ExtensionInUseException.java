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

public class ExtensionInUseException extends UserException {
    private static final String ERROR = "Extension {1} is already in use. "
            + "Please choose another extension for this {0}.";

    public ExtensionInUseException(String objectType, String extension) {
        super(ERROR, objectType, extension);
    }
}
