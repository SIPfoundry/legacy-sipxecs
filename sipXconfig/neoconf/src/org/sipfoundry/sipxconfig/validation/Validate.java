/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.validation;

import java.util.regex.Pattern;

import org.apache.commons.lang.StringUtils;

/**
 * Common validation for commons field types.  Not to used for validating
 * module specific fields like...openacd agent lines... for example.
 */
public final class Validate {
    private static final Pattern IPV4_ADDRESS_BLOCK = Pattern.compile("[0-9]{1,3}(\\.[0-9]{1,3}){0,3}/[0-9]{1,2}");

    private Validate() {
    }

    public static void maxLen(String field, String value, int len) {
        if (StringUtils.isNotBlank(value)) {
            if (value.length() > len) {
                throw new ValidateException("&error.maxLen", field, len);
            }
        }
    }

    public static void ipv4AddrBlock(String field, String value) {
        if (!IPV4_ADDRESS_BLOCK.matcher(value).matches()) {
            throw new ValidateException("&error.invalidIpv4AddrBlock", value);
        }
    }
}
