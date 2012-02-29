/*
 *
 *
 * Copyright (C) 2012 Karel Electronics Corp.
 * All rights reserved.
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
