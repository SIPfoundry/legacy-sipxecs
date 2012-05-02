/**
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
package org.sipfoundry.sipxconfig.feature;

import org.sipfoundry.sipxconfig.common.UserException;

@SuppressWarnings("serial")
public class InvalidChangeException extends UserException {

    public InvalidChangeException(String msg, Object... params) {
        super(msg, params);
    }

    public void appendRawParametersToMessage() {
        Object[] params = getRawParams();
        if (params != null) {
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < params.length; i++) {
                sb.append(" {").append(i).append("}");
            }
            setMessage(getMessage() + sb.toString());
        }
    }
}
