/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */

package org.sipfoundry.openfire.vcard.provider;

public interface ContactInfoHandler {
    public void notifyContactChange(String jabberID);
}
