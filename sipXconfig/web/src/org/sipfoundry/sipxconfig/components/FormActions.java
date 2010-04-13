/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IBinding;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.listener.ListenerInvoker;
import org.apache.tapestry.valid.IValidationDelegate;

public abstract class FormActions extends BaseComponent {
    public static final String OK = "ok";
    public static final String CANCEL = "cancel";
    public static final String APPLY = "apply";
    public static final String REFRESH = "refresh";

    public abstract ICallback getCallback();

    public abstract IActionListener getListener();

    public abstract String getSuccessMessage();

    public abstract SipxValidationDelegate getValidator();

    @InjectObject(value = "infrastructure:listenerInvoker")
    public abstract ListenerInvoker getListenerInvoker();

    public void onOk(IRequestCycle cycle) {
        apply(cycle);
        IValidationDelegate sipxValidator = getSipxValidator();
        if (!sipxValidator.getHasErrors()) {
            getCallback().performCallback(cycle);
        }
    }

    public void setButtonPressedBinding(String buttonId) {
        IBinding binding = getBinding("buttonPressed");
        if (binding != null) {
            binding.setObject(buttonId);
        }
    }

    public void onApply(IRequestCycle cycle) {
        apply(cycle);
    }

    private void apply(IRequestCycle cycle) {
        IActionListener listener = getListener();
        getListenerInvoker().invokeListener(listener, this, cycle);
        SipxValidationDelegate validator = getSipxValidator();
        String msg = StringUtils.defaultIfEmpty(getSuccessMessage(), getMessages().getMessage(
                "user.success"));
        validator.recordSuccess(msg);
    }

    public void onCancel(IRequestCycle cycle) {
        getCallback().performCallback(cycle);
    }

    private SipxValidationDelegate getSipxValidator() {
        SipxValidationDelegate validator = getValidator();
        // for compatibility - we should require passing validator explicitly
        if (validator == null) {
            validator = (SipxValidationDelegate) TapestryUtils.getValidator(getPage());
        }
        return validator;
    }
}
