/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.Iterator;
import java.util.List;

import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;

public class AuthExceptions extends DataSetGenerator {
    private ForwardingContext m_forwardingContext;

    protected DataSet getType() {
        return DataSet.AUTH_EXCEPTION;
    }

    protected void addItems(Element items) {
        List forwardingAliases = m_forwardingContext.getForwardingAuthExceptions();
        for (Iterator i = forwardingAliases.iterator(); i.hasNext();) {
            String exception = (String) i.next();
            Element user = addItem(items).addElement("user");
            user.setText(exception);
        }
    }
    
    public void setForwardingContext(ForwardingContext forwardingContext) {
        m_forwardingContext = forwardingContext;
    }    
}
