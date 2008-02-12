/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import junit.framework.TestCase;

import org.apache.hivemind.util.PropertyUtils;
import org.apache.tapestry.test.Creator;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.admin.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.site.admin.LocalizedLanguageMessages;

public class EditPersonalAttendantTest extends TestCase {

    public void testInitLanguageList() {
        Creator creator = new Creator();
        EditPersonalAttendant out = (EditPersonalAttendant)creator.newInstance(EditPersonalAttendant.class);

        LocalizedLanguageMessages localizedLanguages = org.easymock.classextension.EasyMock.createStrictMock(LocalizedLanguageMessages.class);
        localizedLanguages.setAvailableLanguages(org.easymock.classextension.EasyMock.isA(String[].class));
        localizedLanguages.getMessage("label.pl");
        org.easymock.classextension.EasyMock.expectLastCall().andReturn("Polski");
        org.easymock.classextension.EasyMock.replay(localizedLanguages);
        PropertyUtils.write(out, "localizedLanguageMessages", localizedLanguages);

        LocalizationContext localizationContext = EasyMock.createStrictMock(LocalizationContext.class);
        localizationContext.getInstalledLanguages();
        EasyMock.expectLastCall().andReturn(new String[] {"pl"});
        EasyMock.replay(localizationContext);
        PropertyUtils.write(out, "localizationContext", localizationContext);
        
        out.initLanguageList();
        assertEquals("Polski", out.getLanguageList().getLabel(0));
        assertEquals("pl", out.getLanguageList().getOption(0));
    }
}
