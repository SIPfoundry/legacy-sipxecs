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

import java.util.Arrays;
import java.util.List;
import java.util.Locale;

import junit.framework.TestCase;

import org.apache.hivemind.ApplicationRuntimeException;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.setting.Group;

public class TapestryContextTest extends TestCase {
    private TapestryContext m_context;

    static {
        // Note: this function is called by Tapestry object when first
        // ApplicationRunctimeException is called
        // for some reason it's really slow (couple of seconds)
        // I am calling it here explicitely, not to make it faster, but to better expose the reason
        // why the test is slow
        Locale.getAvailableLocales();
    }

    protected void setUp() throws Exception {
        m_context = new TapestryContext();
    }

    public void testTreatUserExceptionAsValidationError() {
        IMocksControl actionControl = EasyMock.createControl();
        IActionListener action = actionControl.createMock(IActionListener.class);
        action.actionTriggered(null, null);
        actionControl.replay();

        IMocksControl validatorControl = EasyMock.createControl();
        IValidationDelegate validator = validatorControl.createMock(IValidationDelegate.class);
        validatorControl.replay();

        IActionListener listener = m_context.treatUserExceptionAsValidationError(validator,
                action);
        listener.actionTriggered(null, null);

        actionControl.verify();
        validatorControl.verify();
    }

    public void testTreatUserExceptionAsValidationErrorUserException() {
        Throwable exception = new UserException("kuku") {};

        IMocksControl actionControl = EasyMock.createControl();
        IActionListener action = actionControl.createMock(IActionListener.class);
        action.actionTriggered(null, null);
        actionControl.andThrow(new ApplicationRuntimeException(exception));
        actionControl.replay();

        IMocksControl validatorControl = EasyMock.createControl();
        IValidationDelegate validator = validatorControl.createMock(IValidationDelegate.class);
        validator.record((ValidatorException) EasyMock.anyObject());        
        validatorControl.replay();

        IActionListener listener = m_context.treatUserExceptionAsValidationError(validator,
                action);
        listener.actionTriggered(null, null);

        actionControl.verify();
        validatorControl.verify();
    }

    public void testTreatUserExceptionAsValidationErrorOtherException() {
        Throwable exception = new NullPointerException();

        IMocksControl actionControl = EasyMock.createControl();
        IActionListener action = actionControl.createMock(IActionListener.class);
        action.actionTriggered(null, null);
        actionControl.andThrow(new ApplicationRuntimeException(exception));
        actionControl.replay();

        IMocksControl validatorControl = EasyMock.createControl();
        IValidationDelegate validator = validatorControl.createMock(IValidationDelegate.class);
        validatorControl.replay();

        IActionListener listener = m_context.treatUserExceptionAsValidationError(validator,
                action);
        try {
            listener.actionTriggered(null, null);
            fail("ApplicationRuntimeException expected");
        } catch (ApplicationRuntimeException are) {
            assertTrue(are.getCause() instanceof NullPointerException);
        }

        actionControl.verify();
        validatorControl.verify();
    }

    public void testTreatUserExceptionAsValidationErrorNull() {
        IMocksControl actionControl = EasyMock.createControl();
        IActionListener action = actionControl.createMock(IActionListener.class);
        action.actionTriggered(null, null);
        actionControl.andThrow(new ApplicationRuntimeException("kuku"));
        actionControl.replay();

        IMocksControl validatorControl = EasyMock.createControl();
        IValidationDelegate validator = validatorControl.createMock(IValidationDelegate.class);
        validatorControl.replay();

        IActionListener listener = m_context.treatUserExceptionAsValidationError(validator,
                action);
        try {
            listener.actionTriggered(null, null);
            fail("ApplicationRuntimeException expected");
        } catch (ApplicationRuntimeException are) {
            assertNull(are.getCause());
        }

        actionControl.verify();
        validatorControl.verify();
    }
    
    public void joinNamed() {
        Group[] groups = new Group[] { new Group(), new Group()};
        groups[0].setName("robin");
        groups[1].setName("crow");
        List asList = Arrays.asList(groups);
        assertEquals("robin, crow", new TapestryContext().joinNamed(asList, ", "));
    }
}
