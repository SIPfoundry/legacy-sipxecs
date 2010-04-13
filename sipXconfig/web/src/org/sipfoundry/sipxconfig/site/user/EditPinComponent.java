/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.user;


/**
 * Components that edit pin
 *
 * @see UserForm.initializePin and UserForm.updatePin
 */
public interface EditPinComponent {

    // Display this dummy PIN value (masked) to indicate that a PIN exists.
    // We can't use a real PIN.  We don't know the real PIN and if we did,
    // we shouldn't show it.
    // Pick an obscure PIN to avoid colliding with real user PINs.  (I tried using a
    // non-printable PIN "\1\1\1\1\1\1\1\1" but Tapestry silently discards the string!)
    String DUMMY_PIN = "`p1n6P0\361g";

    void setPin(String pin);

    String getPin();
}
