/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import org.dom4j.Element;

/**
 * Transform
 */
public abstract class Transform {
    public Element addToParent(Element parent) {
        Element transform = parent.addElement("transform");
        addChildren(transform);
        return transform;
    }

    protected abstract void addChildren(Element transform);
}
