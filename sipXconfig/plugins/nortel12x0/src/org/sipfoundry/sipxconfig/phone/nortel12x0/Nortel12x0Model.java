/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.nortel12x0;

import org.sipfoundry.sipxconfig.phone.PhoneModel;

public final class Nortel12x0Model extends PhoneModel {

    public Nortel12x0Model() {
        super(Nortel12x0Phone.BEAN_ID);
        setEmergencyConfigurable(true);
    }
}
