/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.moh;

import java.util.Hashtable;

import org.sipfoundry.sipxivr.eslrequest.AbstractEslRequestController;

public class MohEslRequestController extends AbstractEslRequestController {
    private static final String RESOURCE_NAME = "org.sipfoundry.attendant.AutoAttendant";
    private String m_mohParam;

    @Override
    public void extractParameters(Hashtable<String, String> parameters) {
        m_mohParam = parameters.get("moh");
    }

    @Override
    public void loadConfig() {
        initLocalization(RESOURCE_NAME, null);
    }

    public String getMohParam() {
        return m_mohParam;
    }

}
