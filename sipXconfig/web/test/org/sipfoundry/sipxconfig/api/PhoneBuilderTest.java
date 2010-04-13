/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.api;

import junit.framework.TestCase;

public class PhoneBuilderTest extends TestCase {

    public void testToApi() {
        PhoneBuilder builder = new PhoneBuilder();
        org.sipfoundry.sipxconfig.phone.Phone myPhone = new org.sipfoundry.sipxconfig.phone.acme.AcmePhone();
        Phone apiPhone = new Phone();
        ApiBeanUtil.toApiObject(builder, apiPhone, myPhone);
        assertEquals(apiPhone.getModelId(), myPhone.getModelId());
    }
}
