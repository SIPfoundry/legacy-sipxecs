/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.commons.security;

import static org.apache.commons.lang.StringUtils.split;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Util {
    private static final Pattern MD5_HASH_PATTERN = Pattern.compile("(?i)(?<![a-z0-9])[a-f0-9]{32}(?![a-z0-9])");

    public static String retrieveUsername(String loginEntry) {
        String[] userArray = split(loginEntry, '@');
        String userLoginName = loginEntry;
        if (userArray.length == 2) {
            userLoginName = userArray[0];
        } else {
            userArray = split(loginEntry, '\\');
            if (userArray.length == 2) {
                userLoginName = userArray[1];
            }
        }
        return userLoginName;
    }

    public static String retrieveDomain(String loginEntry) {
        String[] userArray = split(loginEntry, '@');
        String domain = null;
        if (userArray.length == 2) {
            domain = userArray[1];
        } else {
            userArray = split(loginEntry, '\\');
            if (userArray.length == 2) {
                domain = userArray[0];
            }
        }
        return domain;
    }

    public static boolean isHashed(String hashedString) {
        Matcher matcher = MD5_HASH_PATTERN.matcher(hashedString);
        return matcher.find();
    }
}
