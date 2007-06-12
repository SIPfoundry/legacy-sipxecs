/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * 
 */
package org.sipfoundry.sipxconfig.site.common;

import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.annotations.Parameter;

public abstract class ListPanel extends BaseComponent {
    @Parameter(required = true)
    public abstract List getSource();

    @Parameter(required = true)
    public abstract IActionListener getAddListener();

    @Parameter(required = true)
    public abstract IActionListener getDeleteListener();

    @Parameter(required = true)
    public abstract String getLabel();

    public abstract String getItemDisplayName();
    
    @Parameter(defaultValue = "message:button.add")
    public abstract String getAddLinkLabel();
    
    public abstract int getIndex();

    public Object getValue() {
        return getSource().get(getIndex());
    }

    public void setValue(Object value) {
        getSource().set(getIndex(), value);
    }
}
