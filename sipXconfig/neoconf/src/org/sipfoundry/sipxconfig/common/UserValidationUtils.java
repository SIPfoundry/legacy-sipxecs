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

import java.util.regex.Matcher;
import java.util.regex.Pattern;

public final class UserValidationUtils {

    private static final Pattern VALID_USER_NAME = Pattern.compile("([-_.!~*'\\(\\)&amp;=+$,;?/"
            + "a-zA-Z0-9]|(&#37;[0-9a-fA-F]{2}))+");

    private static final Pattern EMAIL_PATTERN = Pattern.compile("^[_A-Za-z0-9-\\+]+(\\.[_A-Za-z0-9-]+)*@"
            + "[A-Za-z0-9-]+(\\.[A-Za-z0-9]+)*(\\.[A-Za-z]{2,})$");

    private UserValidationUtils() {
        // Utility class - do not instantiate
    }

    public static boolean isValidUserName(String userName) {
        if (userName == null) {
            return false;
        }
        Matcher m = VALID_USER_NAME.matcher(userName);
        return m.matches();
    }

    public static boolean isValidEmail(String email) {
        if (email == null) {
            return false;
        }
        Matcher m = EMAIL_PATTERN.matcher(email);
        return m.matches();
    }
}
