/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.common.ExtensionPool;
import org.sipfoundry.sipxconfig.common.ExtensionPoolContext;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class ExtensionPoolsPage extends PageWithCallback {

    public static final String PAGE = "admin/ExtensionPools";

    @InjectObject(value = "spring:extensionPoolContext")
    public abstract ExtensionPoolContext getExtensionPoolContext();

    @InitialValue(value = "ognl:extensionPoolContext.userExtensionPool")
    public abstract ExtensionPool getUserExtensionPool();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public void commit() {
        // Proceed only if Tapestry validation succeeded
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        // Complain if the extension pool upper limit is lower than the lower limit
        ExtensionPool pool = getUserExtensionPool();
        if (pool.getFirstExtension() != null && pool.getLastExtension() != null
                && pool.getLastExtension() < pool.getFirstExtension()) {
            recordError("message.lastExtensionTooSmall");
            return;
        }

        // For now, we are not letting the user edit the pool's nextExtension value,
        // which controls where to start looking for the next free extension.
        // Set it to be the same as the start of the range.
        pool.setNextExtension(pool.getFirstExtension());

        getExtensionPoolContext().saveExtensionPool(pool);
    }

    private void recordError(String messageId) {
        IValidationDelegate delegate = TapestryUtils.getValidator(getPage());
        delegate.record(getMessages().getMessage(messageId), ValidationConstraint.TOO_SMALL);
    }
}
