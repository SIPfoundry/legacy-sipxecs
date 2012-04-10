/*
 *
 *
 * Copyright (C) 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.commons.util;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class SipUriUtil {
    private static final Pattern EXTRACT_USER_RE = Pattern.compile("\\s*<?(?:sip:)?(.+?)[@;].+");
    private static final Pattern EXTRACT_FULL_USER_RE = Pattern
            .compile("\\s*(?:\"?\\s*([^\"<]+?)\\s*\"?)?\\s*<(?:sip:)?(.+?)[@;].+");

    /**
     * Extracts user name if available. Otherwise it returns the user id
     */
    public static String extractUserName(String uri) {
        if (uri == null) {
            return null;
        }
        Matcher matcher = EXTRACT_FULL_USER_RE.matcher(uri);
        if (!matcher.matches()) {
            matcher = EXTRACT_USER_RE.matcher(uri);
            if (matcher.matches()) {
                return matcher.group(1);
            }
            return null;
        }
        String fullName = matcher.group(1);
        String userId = matcher.group(2);

        if (fullName == null || fullName.equals(userId)) {
            return userId;
        }
        return fullName;
    }
}
