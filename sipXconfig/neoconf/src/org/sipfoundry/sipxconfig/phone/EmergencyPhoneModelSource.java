/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import org.apache.commons.collections.Predicate;
import org.sipfoundry.sipxconfig.device.FilteredModelSource;

public class EmergencyPhoneModelSource extends FilteredModelSource<PhoneModel> {

    public EmergencyPhoneModelSource() {
        setFilter(getPredicate());
    }

    static Predicate getPredicate() {
        return new Predicate() {
            public boolean evaluate(Object arg0) {
                return ((PhoneModel) arg0).isEmergencyConfigurable();
            }
        };
    }
}
