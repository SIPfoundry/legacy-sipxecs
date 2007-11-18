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
