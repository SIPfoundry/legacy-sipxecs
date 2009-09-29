/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.presence;

import java.util.Hashtable;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;

public class PresenceServerTest extends TestCase {

    private static final Hashtable SUCCESS_RESPONSE = new Hashtable();
    static {
        SUCCESS_RESPONSE.put("result-code", new Integer(1));
    }

    public void testSignInAction() {
        IMocksControl coreContextCtrl = EasyMock.createStrictControl();
        CoreContext coreContext = coreContextCtrl.createMock(CoreContext.class);
        coreContext.getDomainName();
        coreContextCtrl.andReturn("example.com");
        coreContextCtrl.replay();

        Hashtable signin = new Hashtable();
        signin.put("object-class", "login");
        signin.put("user-action", "sip:joe@example.com");

        IMocksControl apiCtrl = EasyMock.createControl();
        PresenceServerImpl.SignIn api = apiCtrl.createMock(PresenceServerImpl.SignIn.class);
        api.action(signin);
        apiCtrl.andReturn(SUCCESS_RESPONSE);
        apiCtrl.replay();

        User joe = new User();
        joe.setUserName("joe");

        PresenceServerImpl impl = new PresenceServerImpl();
        impl.setCoreContext(coreContext);
        impl.userAction(api, "user-action", joe);

        apiCtrl.verify();
        coreContextCtrl.verify();
    }

    public void testCheckError() {
        Hashtable response = new Hashtable();
        response.put("result-code", new Integer(0));
        response.put("result-text", "testing error handling");
        try {
            PresenceServerImpl.checkErrorCode(response);
            fail("Expected exception");
        } catch (UserException e) {
            assertTrue(true);
        }

        PresenceServerImpl.checkErrorCode(SUCCESS_RESPONSE);
    }
}
