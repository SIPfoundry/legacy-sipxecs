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

import java.util.Locale;

import junit.framework.JUnit4TestAdapter;

import org.apache.hivemind.ApplicationRuntimeException;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.listener.ListenerInvoker;
import org.apache.tapestry.listener.ListenerInvokerFilter;
import org.apache.tapestry.test.Creator;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.junit.Test;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.site.user.EditUser;

import static org.easymock.EasyMock.anyObject;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.assertEquals;

public class UserExceptionFilterTest {
    static {
        // Note: this function is called by Tapestry object when first
        // ApplicationRunctimeException is called
        // for some reason it's really slow (couple of seconds)
        // I am calling it here explicitly, not to make it faster, but to better expose the reason
        // why the test is slow
        Locale.getAvailableLocales();
    }

    public static junit.framework.Test suite() {
        return new JUnit4TestAdapter(UserExceptionFilterTest.class);
    }

    @Test
    public void testNoException() {
        IActionListener action = createMock(IActionListener.class);
        IValidationDelegate validator = createMock(IValidationDelegate.class);

        ListenerInvoker delegate = createMock(ListenerInvoker.class);
        delegate.invokeListener(action, null, null);
        expectLastCall();

        replay(action, validator, delegate);

        ListenerInvokerFilter filter = new UserExceptionFilter();
        filter.invokeListener(action, null, null, delegate);

        verify(action, validator, delegate);
    }

    @Test
    public void testUserException() {
        IActionListener action = createMock(IActionListener.class);
        final IValidationDelegate validator = createNiceMock(IValidationDelegate.class);
        validator.record((ValidatorException) anyObject());

        ListenerInvoker delegate = createMock(ListenerInvoker.class);
        delegate.invokeListener(action, null, null);
        Throwable throwable = new UserException("error");
        expectLastCall().andThrow(throwable);

        replay(action, validator, delegate);

        ListenerInvokerFilter filter = new UserExceptionFilter() {
            IValidationDelegate findValidator(IComponent source) {
                return validator;
            }
        };
        filter.invokeListener(action, null, null, delegate);

        verify(action, validator, delegate);
    }

    @Test
    public void testApplicationException() {
        IPage source = (IPage) new Creator().newInstance(EditUser.class);
        source.setPage(source);

        IActionListener action = createMock(IActionListener.class);

        ListenerInvoker delegate = createMock(ListenerInvoker.class);
        delegate.invokeListener(action, source, null);
        Throwable ue = new UserException("error");
        ApplicationRuntimeException exception = new ApplicationRuntimeException(ue);
        expectLastCall().andThrow(exception);

        replay(action, delegate);

        final SipxValidationDelegate validator = new SipxValidationDelegate();
        ListenerInvokerFilter filter = new UserExceptionFilter() {
            IValidationDelegate findValidator(IComponent _source) {
                return validator;
            }
        };
        filter.invokeListener(action, source, null, delegate);

        assertEquals("error", validator.getFirstError().toString());
        verify(action, delegate);
    }

    @Test(expected = NullPointerException.class)
    public void testAnotherException() {
        ListenerInvoker delegate = createMock(ListenerInvoker.class);
        delegate.invokeListener(null, null, null);
        RuntimeException exception = new NullPointerException();
        expectLastCall().andThrow(exception);

        replay(delegate);

        ListenerInvokerFilter filter = new UserExceptionFilter();
        filter.invokeListener(null, null, null, delegate);
    }

    @Test(expected = ApplicationRuntimeException.class)
    public void testAnotherWrappedException() {
        ListenerInvoker delegate = createMock(ListenerInvoker.class);
        delegate.invokeListener(null, null, null);
        Throwable ue = new NullPointerException();
        ApplicationRuntimeException exception = new ApplicationRuntimeException(ue);
        expectLastCall().andThrow(exception);

        replay(delegate);

        ListenerInvokerFilter filter = new UserExceptionFilter();
        filter.invokeListener(null, null, null, delegate);
    }
}
