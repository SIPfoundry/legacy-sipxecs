/*
*
*
* Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
* Contributors retain copyright to elements licensed under a Contributor Agreement.
* Licensed to the User under the LGPL license.
*
* $
*/
package org.sipfoundry.sipxconfig.site.common;

import junit.framework.TestCase;

import org.apache.hivemind.Messages;
import org.apache.hivemind.util.PropertyUtils;
import org.apache.tapestry.test.Creator;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.admin.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.site.admin.LocalizedLanguageMessages;
import org.sipfoundry.sipxconfig.site.common.InstalledLanguages;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.isA;
import static org.easymock.classextension.EasyMock.replay;
import static org.easymock.classextension.EasyMock.createStrictMock;

public class InstalledLanguagesTest extends TestCase {

   public void testInitLanguageList() {
       Creator creator = new Creator();
       InstalledLanguages out = (InstalledLanguages) creator.newInstance(InstalledLanguages.class);

       Messages messages = createMock(Messages.class);
       messages.getMessage("label.default");
       expectLastCall().andReturn("Default");

       replay(messages);
       PropertyUtils.write(out, "messages", messages);

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

       out.initLanguages();
       assertEquals("Default", out.getLanguagesSelectionModel().getLabel(0));
       assertEquals(null, out.getLanguagesSelectionModel().getOption(0));
       assertEquals("Polski", out.getLanguagesSelectionModel().getLabel(1));
       assertEquals("pl", out.getLanguagesSelectionModel().getOption(1));

   }
}
