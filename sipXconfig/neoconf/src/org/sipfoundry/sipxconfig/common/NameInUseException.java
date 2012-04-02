/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;


public class NameInUseException extends UserException {
    private static final String ERROR_LONG = "&error.nameInUse.long";

    private static final String ERROR_SHORT = "&error.nameInUse.short";

    public NameInUseException(String objectType, String name) {
        super(ERROR_LONG, objectType, name);
    }

    public NameInUseException(String name) {
        super(ERROR_SHORT, name);
    }
}
