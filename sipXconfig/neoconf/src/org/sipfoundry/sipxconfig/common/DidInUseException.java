/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;


public class DidInUseException extends UserException {
    private static final String ERROR = "&error.didInUse";

    public DidInUseException(String objectType, String extension) {
        super(ERROR, objectType, extension);
    }
}
