/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.components;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hivemind.ApplicationRuntimeException;
import org.apache.hivemind.Messages;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.listener.ListenerInvoker;
import org.apache.tapestry.listener.ListenerInvokerFilter;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.common.UserException;

public class UserExceptionFilter implements ListenerInvokerFilter {
    private static final Log LOG = LogFactory.getLog(UserExceptionFilter.class);

    public void invokeListener(IActionListener listener, IComponent source, IRequestCycle cycle,
            ListenerInvoker delegate) {
        try {
            delegate.invokeListener(listener, source, cycle);
        } catch (ApplicationRuntimeException are) {
            UserException cause = getUserExceptionCause(are);
            if (cause != null) {
                recordUserException(cause, source);
            } else {
                throw are;
            }
        } catch (UserException ue) {
            recordUserException(ue, source);
        }
    }

    /**
     * Starting with Tapestry 4, Listeners wrap exceptions with ApplicationRuntimeException. We
     * have to prepare for many levels of exceptions as listeners are often wrapped by other
     * listeners
     */
    private UserException getUserExceptionCause(ApplicationRuntimeException e) {
        Throwable t = e.getCause();
        if (t instanceof UserException) {
            return (UserException) t;
        }
        if (t instanceof ApplicationRuntimeException && t != e) {
            // recurse
            return getUserExceptionCause((ApplicationRuntimeException) t);
        }
        return null;
    }

    private void recordUserException(UserException e, IComponent source) {
        IValidationDelegate validator = findValidator(source);
        if (validator instanceof SipxValidationDelegate) {
            Messages messages = source.getPage().getMessages();
            SipxValidationDelegate sipxValidator = (SipxValidationDelegate) validator;
            sipxValidator.record(e, messages);
        } else if (validator != null) {
            validator.record(new ValidatorException(e.getMessage()));
        } else {
            LOG.warn("Cannot record exception for page: <" + source.getPage().getPageName() + ">");
            throw e;
        }
    }

    /**
     * Locate validator form source.
     *
     * It is in a separate method mostly to make testing possible. Standard implementation is
     * using our trusted TapestryUtil hack.
     */
    IValidationDelegate findValidator(IComponent source) {
        return TapestryUtils.getValidator(source);
    }
}
