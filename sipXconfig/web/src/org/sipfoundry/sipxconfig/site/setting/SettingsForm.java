/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.setting;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IForm;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.valid.IValidationDelegate;

public abstract class SettingsForm extends BaseComponent {

    public abstract IActionListener getAction();

    public abstract IActionListener getCancelListener();

    /**
     * Called when any of the submit componens on the form is activated.
     *
     * Check if cancel was pressed and if so call the cancel action. Otherwise verify if form is
     * valid and only call the cancel action if that's the case.
     *
     *
     * @param cycle current request cycle
     */
    public void formSubmit(IRequestCycle cycle) {
        IActionListener action = getAction();
        IValidationDelegate delegate = getValidator();
        if (null == action) {
            delegate.clear();
            return;
        }
        if (action == getCancelListener()) {
            // clear errors before calling cancel
            delegate.clear();
        }
        if (!delegate.getHasErrors()) {
            action.actionTriggered(this, cycle);
        }
    }

    private IValidationDelegate getValidator() {
        IForm form = (IForm) getComponent("settingsForm");
        IValidationDelegate delegate = form.getDelegate();
        return delegate;
    }
}
