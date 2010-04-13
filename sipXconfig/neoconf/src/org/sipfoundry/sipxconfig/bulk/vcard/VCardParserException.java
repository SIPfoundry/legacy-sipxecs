/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.bulk.vcard;

import org.sipfoundry.sipxconfig.common.UserException;

public class VCardParserException extends UserException {
    public VCardParserException() {
        super("&msg.phonebookVcardUploadError");
    }
}
