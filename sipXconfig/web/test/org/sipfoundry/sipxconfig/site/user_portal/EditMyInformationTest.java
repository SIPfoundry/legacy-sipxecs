package org.sipfoundry.sipxconfig.site.user_portal;

import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

import org.apache.hivemind.util.PropertyUtils;
import org.apache.tapestry.test.Creator;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.admin.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.site.common.LanguageSupport;

public class EditMyInformationTest extends TestCase {
    
    public void testLanguageLocalization() {
        Creator creator = new Creator();
        EditMyInformation out = (EditMyInformation)creator.newInstance(EditMyInformation.class);

        LanguageSupport languageSupport = EasyMock.createStrictMock(LanguageSupport.class);
        languageSupport.resolveLocaleName("fr");
        EasyMock.expectLastCall().andReturn("français");
        languageSupport.resolveLocaleName("fr-CA");
        EasyMock.expectLastCall().andReturn("French (Canada)");
        EasyMock.replay(languageSupport);
        PropertyUtils.write(out, "languageSupport", languageSupport);
        
        LocalizationContext localizationContext = EasyMock.createStrictMock(LocalizationContext.class);
        localizationContext.getInstalledLanguages();
        EasyMock.expectLastCall().andReturn(new String[] {"fr", "fr-CA"});
        EasyMock.replay(localizationContext);
        PropertyUtils.write(out, "localizationContext", localizationContext);
        
        out.initLanguageList();
        
        List<String> languages = new ArrayList<String>();
        int numLanguages = out.getLanguageList().getOptionCount();
        for (int i = 0; i < numLanguages; i++) {
            languages.add(out.getLanguageList().getLabel(i));
        }
        
        assertTrue(languages.contains("français"));
        assertTrue(languages.contains("French (Canada)"));
    }
}
