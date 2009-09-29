/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.io.File;
import java.io.IOException;
import java.util.Collection;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;


public class PatternSettingFilterTest extends TestCase {

    public void testPatternFilter() throws IOException {
        ModelBuilder builder = new XmlModelBuilder("etc");
        File in = TestHelper.getResourceAsFile(getClass(), "games.xml");
        SettingSet games = builder.buildModel(in);

        PatternSettingFilter filter = new PatternSettingFilter();
        filter.addExcludes("cards.*$");
        filter.addExcludes("chess/piece.*$");
        Collection settings = SettingUtil.filter(filter, games);
        assertEquals(4, settings.size());
    }
}
