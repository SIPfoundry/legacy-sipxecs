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


public class ExtensionInUseException extends UserException {
    private static final String ERROR = "&error.extensionInUse";

    public ExtensionInUseException(String objectType, String extension) {
        super(ERROR, objectType, extension);
    }
}
