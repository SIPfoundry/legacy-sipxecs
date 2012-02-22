/*
 *
 *
 * Copyright (C) 2011 Karel Electronics Corp.
 * All rights reserved.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import org.sipfoundry.sipxconfig.common.UserException;

public class DidInUseException extends UserException {
    private static final String ERROR = "&error.didInUse";

    public DidInUseException(String objectType, String extension) {
        super(ERROR, objectType, extension);
    }
}
