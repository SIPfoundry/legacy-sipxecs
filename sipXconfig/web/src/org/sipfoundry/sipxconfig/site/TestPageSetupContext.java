/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site;

/**
 * Test settings when running unit tests
 */
public class TestPageSetupContext {
    private String m_mailstoreSample;

    public String getMailstoreSample() {
        return m_mailstoreSample;
    }

    public void setMailstoreSample(String mailstoreSample) {
        m_mailstoreSample = mailstoreSample;
    }
}
