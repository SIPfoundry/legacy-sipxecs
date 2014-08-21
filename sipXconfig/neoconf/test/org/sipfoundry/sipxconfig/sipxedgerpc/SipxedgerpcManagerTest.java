/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.sipxedgerpc;

import static org.junit.Assert.assertEquals;

import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class SipxedgerpcManagerTest {
    @Test
    public void testConfig() throws Exception {
        StringWriter actual = new StringWriter();
        SipxedgerpcManagerSettings settings = new SipxedgerpcManagerSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        settings.setSettingTypedValue("rpc-config/rpc-http-port", 333);
        settings.setSettingTypedValue("rpc-config/log-level", 4);
        SipxedgerpcManagerImpl.writeConfig(actual, settings);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-sipxedgerpc-config"));
        assertEquals(expected, actual.toString());
    }
}
