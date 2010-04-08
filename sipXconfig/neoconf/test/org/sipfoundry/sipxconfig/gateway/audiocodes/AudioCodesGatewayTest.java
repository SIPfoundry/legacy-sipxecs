/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import junit.framework.TestCase;

public class AudioCodesGatewayTest extends TestCase {

    public void testGetProfileFilename() {
        AudioCodesGateway gateway = new AudioCodesGateway() {
            @Override
            int getMaxCalls() {
                return 0;
            }
        };

        gateway.setSerialNumber("00908f03f59B");
        assertEquals("00908F03F59B.ini", gateway.getProfileFilename());

        gateway.setSerialNumber(null);
        assertNull(gateway.getProfileFilename());
    }

    public void testDefaultDeviceVersion() {
        AudioCodesGateway gateway = new AudioCodesGateway() {
            @Override
            int getMaxCalls() {
                return 0;
            }
        };

        assertEquals("6.0", gateway.getDeviceVersion().getVersionId());
    }
}
