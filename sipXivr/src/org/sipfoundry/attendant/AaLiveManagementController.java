/**
 *
 *
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.attendant;

import java.util.Hashtable;

import org.sipfoundry.commons.freeswitch.Collect;
import org.sipfoundry.sipxivr.eslrequest.AbstractEslRequestController;

public class AaLiveManagementController extends AbstractEslRequestController {
    private String m_dialedNumber;

    @Override
    public void extractParameters(Hashtable<String, String> parameters) {
        m_dialedNumber = parameters.get("dialed");
    }

    public String getDialedNumber() {
        return m_dialedNumber;
    }

    @Override
    public void loadConfig() {
        initLocalization("AutoAttendant", "org.sipfoundry.attendant.AutoAttendant");
    }

    public String promptForCode() {
        Collect c = new Collect(getFsEventSocket(), 20, 10000, 1000, 1000);
        c.setTermChars("#");
        c.go();
        return c.getDigits();
    }
}
