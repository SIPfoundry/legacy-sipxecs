/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.site.common.LanguageSupport;

public class LocalizedLanguageMessagesTest extends TestCase {
    private static final String LABEL = "label.";
    private static final String FRENCH_CANADA = "French (Canada)";
    private static final String POLSKI = "polski";
    private static final String FR_CA = "fr-CA";
    private static final String PL = "pl";

    public void testFindMessage() {
        LocalizedLanguageMessages out = new LocalizedLanguageMessages();

        LanguageSupport languageSupport = EasyMock.createStrictMock(LanguageSupport.class);
        languageSupport.resolveLocaleName(PL);
        EasyMock.expectLastCall().andReturn(POLSKI);
        languageSupport.resolveLocaleName(FR_CA);
        EasyMock.expectLastCall().andReturn(FRENCH_CANADA);
        EasyMock.replay(languageSupport);

        out.setLanguageSupport(languageSupport);
        out.setAvailableLanguages(new String[] {PL, FR_CA});

        assertEquals(POLSKI, out.getMessage(PL));
        assertEquals(FRENCH_CANADA, out.getMessage(FR_CA));
    }

    public void testFindMessageWithLabelString() {
        LocalizedLanguageMessages out = new LocalizedLanguageMessages();

        LanguageSupport languageSupport = EasyMock.createStrictMock(LanguageSupport.class);
        languageSupport.resolveLocaleName(PL);
        EasyMock.expectLastCall().andReturn(POLSKI);
        languageSupport.resolveLocaleName(FR_CA);
        EasyMock.expectLastCall().andReturn(FRENCH_CANADA);
        EasyMock.replay(languageSupport);

        out.setLanguageSupport(languageSupport);
        out.setAvailableLanguages(new String[] {PL, FR_CA});

        assertEquals(POLSKI, out.getMessage(LABEL + PL));
        assertEquals(FRENCH_CANADA, out.getMessage(LABEL + FR_CA));
    }
}
