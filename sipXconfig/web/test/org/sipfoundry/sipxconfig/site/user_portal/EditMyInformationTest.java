/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.user_portal;

import junit.framework.TestCase;

import org.apache.hivemind.util.PropertyUtils;
import org.apache.tapestry.test.Creator;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.admin.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.site.admin.LocalizedLanguageMessages;

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.isA;
import static org.easymock.classextension.EasyMock.replay;
import static org.easymock.classextension.EasyMock.createStrictMock;

public class EditMyInformationTest extends TestCase {

    public void testInitLanguageList() {
        Creator creator = new Creator();
        EditMyInformation out = (EditMyInformation) creator.newInstance(EditMyInformation.class);

        LocalizedLanguageMessages localizedLanguages = createStrictMock(LocalizedLanguageMessages.class);
        localizedLanguages.setAvailableLanguages(isA(String[].class));
        localizedLanguages.getMessage("label.pl");
        expectLastCall().andReturn("Polski");
        replay(localizedLanguages);
        PropertyUtils.write(out, "localizedLanguageMessages", localizedLanguages);

        LocalizationContext localizationContext = EasyMock.createStrictMock(LocalizationContext.class);
        localizationContext.getInstalledLanguages();
        expectLastCall().andReturn(new String[] {
            "pl"
        });
        replay(localizationContext);
        PropertyUtils.write(out, "localizationContext", localizationContext);

        out.initLanguageList();
        assertEquals("Polski", out.getLanguageList().getLabel(0));
        assertEquals("pl", out.getLanguageList().getOption(0));
    }
}
