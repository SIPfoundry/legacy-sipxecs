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

    private DuplicateEntity m_duplicateEntity;

    public NameInUseException(String objectType, String name) {
        super(ERROR_LONG, objectType, name);
    }

    public NameInUseException(DuplicateEntity duplicateEntity) {
        super(ERROR_SHORT, duplicateEntity.getValue());
        m_duplicateEntity = duplicateEntity;
    }

    public DuplicateEntity getDuplicateEntity() {
        return m_duplicateEntity;
    }
}
