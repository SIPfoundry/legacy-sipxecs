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
package org.sipfoundry.commons.mongo;


/**
 * mongo resync result wrapper
 * 
 */
public class ResyncResult {
    private double m_status;
    private Object m_errorMessage;

    public ResyncResult(double status, Object errorMsg) {
        m_status = status;
        m_errorMessage = errorMsg;
    }

    public double getStatus() {
        return m_status;
    }

    public Object getErrorMessage() {
        return m_errorMessage;
    }

}
