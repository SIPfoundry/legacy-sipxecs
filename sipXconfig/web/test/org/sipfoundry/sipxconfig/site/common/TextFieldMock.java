/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.common;

import org.apache.tapestry.IForm;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.AbstractFormComponent;

public class TextFieldMock extends AbstractFormComponent {

    @Override
    public IForm getForm() {
        return null;
    }

    @Override
    public String getIdParameter() {
        return null;
    }

    @Override
    public String getName() {
        return null;
    }

    @Override
    protected void renderFormComponent(IMarkupWriter arg0, IRequestCycle arg1) {
    }

    @Override
    protected void rewindFormComponent(IMarkupWriter arg0, IRequestCycle arg1) {
    }

    @Override
    public void setForm(IForm arg0) {
    }

    @Override
    public void setName(String arg0) {
    }

    @Override
    public String getClientId() {
        return null;
    }

    @Override
    public void setClientId(String arg0) {
    }

    @Override
    public String getDisplayName() {
        return null;
    }

    @Override
    public boolean isDisabled() {
        return false;
    }

}
