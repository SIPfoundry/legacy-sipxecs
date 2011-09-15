/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
