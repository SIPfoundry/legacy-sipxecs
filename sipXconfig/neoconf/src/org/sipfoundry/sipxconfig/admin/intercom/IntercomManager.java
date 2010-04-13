/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.intercom;

import java.util.List;

import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleProvider;
import org.sipfoundry.sipxconfig.common.DataObjectSource;
import org.sipfoundry.sipxconfig.phone.Phone;

public interface IntercomManager extends DataObjectSource, DialingRuleProvider {

    public Intercom newIntercom();

    /**
     * Return an Intercom instance. Create one if none exist. Throw UserException
     * if more than one Intercom instance exists, since the caller is assuming that
     * there can be at most one Intercom instance.
     */
    public Intercom getIntercom();

    public void saveIntercom(Intercom intercom);

    public List<Intercom> loadIntercoms();

    public void clear();

    /**
     * Return the intercom associated with a phone, through the groups the phone
     * belongs to, or null if there is no intercom for the phone.
     * There should be at most one intercom for any phone. If there is more than
     * one, then return the first intercom found.
     */
    public Intercom getIntercomForPhone(Phone phone);

}
