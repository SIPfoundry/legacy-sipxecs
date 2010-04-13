/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin.localization;

import java.util.Arrays;
import java.util.List;

import junit.framework.TestCase;

public class LocalizationContextImplTest extends TestCase {

    public void testGetInstalledLanguages() {
        LocalizationContextImpl out = new LocalizationContextImpl() {
            protected String[] getListOfDirectories(String path, String prefix) {
                return new String[] {"stdprompts_en", "stdprompts_de"};
            }
        };

        List<String> languageList = Arrays.<String>asList(out.getInstalledLanguages());
        assertTrue(languageList.contains("en"));
        assertTrue(languageList.contains("de"));
    }
}
