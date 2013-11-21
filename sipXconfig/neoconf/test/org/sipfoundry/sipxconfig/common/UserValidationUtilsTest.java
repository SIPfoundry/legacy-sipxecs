/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.common;

import junit.framework.TestCase;

public class UserValidationUtilsTest extends TestCase {

    public void testUserValid() {
        assertEquals(false, UserValidationUtils.isValidUserName("@200"));
        assertEquals(true, UserValidationUtils.isValidUserName("200"));
    }

    public void testEmailValid() {
        assertEquals(false, UserValidationUtils.isValidEmail("@200"));
        assertEquals(true, UserValidationUtils.isValidEmail("200@test.ro"));
    }
}
