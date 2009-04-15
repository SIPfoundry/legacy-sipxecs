/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.site.admin.ldap;

import java.util.Collection;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class LdapObjectClassPanel extends BaseComponent {

    @Parameter
    public abstract String[] getObjectClasses();

    @Parameter
    public abstract Collection<String> getSelectedObjectClasses();

    public abstract IPropertySelectionModel getModelCached();

    public abstract void setModelCached(IPropertySelectionModel model);

    public IPropertySelectionModel getModel() {
        IPropertySelectionModel model = getModelCached();
        if (model != null) {
            return model;
        }
        model = new StringPropertySelectionModel(getObjectClasses());
        setModelCached(model);
        return model;
    }
}
