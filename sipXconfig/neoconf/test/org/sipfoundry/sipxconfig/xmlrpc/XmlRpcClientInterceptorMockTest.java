/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.xmlrpc;

import java.net.ConnectException;

import junit.framework.TestCase;

import org.apache.xmlrpc.XmlRpcClient;
import org.apache.xmlrpc.XmlRpcClientRequest;
import org.apache.xmlrpc.XmlRpcException;
import org.easymock.IMocksControl;
import org.easymock.classextension.EasyMock;
import org.springframework.aop.framework.ProxyFactory;

public class XmlRpcClientInterceptorMockTest extends TestCase {

    public void testIntercept() throws Exception {
        IMocksControl mcClient = EasyMock.createControl();
        XmlRpcClient client = mcClient.createMock(XmlRpcClient.class);
        client.execute(anyRequest());
        mcClient.andReturn("xxxxx");
        mcClient.replay();

        XmlRpcClientInterceptor interceptor = new XmlRpcClientInterceptor();
        interceptor.setServiceInterface(TestFunctions.class);
        interceptor.setXmlRpcClient(client);

        TestFunctions proxy = (TestFunctions) ProxyFactory.getProxy(TestFunctions.class,
                interceptor);

        String result = proxy.multiplyTest("x", 5);
        assertEquals("xxxxx", result);

        mcClient.verify();
    }

    public void testInterceptException() throws Exception {
        IMocksControl mcClient = org.easymock.classextension.EasyMock.createControl();
        XmlRpcClient client = mcClient.createMock(XmlRpcClient.class);
        client.execute(anyRequest());
        mcClient.andThrow(new XmlRpcException(2, "message"));
        mcClient.replay();

        XmlRpcClientInterceptor interceptor = new XmlRpcClientInterceptor();
        interceptor.setServiceInterface(TestFunctions.class);
        interceptor.setXmlRpcClient(client);

        TestFunctions proxy = (TestFunctions) ProxyFactory.getProxy(TestFunctions.class,
                interceptor);

        try {
            proxy.multiplyTest("x", 5);
            fail("Should throw exception");
        } catch (XmlRpcRemoteException e) {
            assertEquals(2, e.getFaultCode());
            assertEquals("message", e.getMessage());
        }

        mcClient.verify();
    }

    public void testInterceptRuntimeException() throws Exception {
        IMocksControl mcClient = org.easymock.classextension.EasyMock.createControl();
        XmlRpcClient client = mcClient.createMock(XmlRpcClient.class);
        client.execute(anyRequest());
        mcClient.andThrow(new NullPointerException());
        mcClient.replay();

        XmlRpcClientInterceptor interceptor = new XmlRpcClientInterceptor();
        interceptor.setServiceInterface(TestFunctions.class);
        interceptor.setXmlRpcClient(client);

        TestFunctions proxy = (TestFunctions) ProxyFactory.getProxy(TestFunctions.class,
                interceptor);

        try {
            proxy.multiplyTest("x", 5);
            fail("Should throw exception");
        } catch (NullPointerException e) {
            // ok
        }

        mcClient.verify();
    }

    public void testInterceptConnectionException() throws Exception {
        IMocksControl mcClient = org.easymock.classextension.EasyMock.createControl();
        XmlRpcClient client = mcClient.createMock(XmlRpcClient.class);
        client.execute(anyRequest());
        mcClient.andThrow(new ConnectException("test"));
        mcClient.replay();

        XmlRpcClientInterceptor interceptor = new XmlRpcClientInterceptor();
        interceptor.setServiceInterface(TestFunctions.class);
        interceptor.setXmlRpcClient(client);

        TestFunctions proxy = (TestFunctions) ProxyFactory.getProxy(TestFunctions.class,
                interceptor);

        try {
            proxy.multiplyTest("x", 5);
            fail("Should throw exception");
        } catch (XmlRpcRemoteException e) {
            assertEquals(0, e.getFaultCode());
            assertEquals("test", e.getMessage());
            assertTrue(e.getCause() instanceof ConnectException);
        }

        mcClient.verify();
    }

    public void testInterceptFault() throws Exception {
        IMocksControl mcClient = org.easymock.classextension.EasyMock.createControl();
        XmlRpcClient client = mcClient.createMock(XmlRpcClient.class);
        client.execute(anyRequest());
        // sometimes client will return exception instead of throwing it
        mcClient.andReturn(new XmlRpcException(2, "message"));
        mcClient.replay();

        XmlRpcClientInterceptor interceptor = new XmlRpcClientInterceptor();
        interceptor.setServiceInterface(TestFunctions.class);
        interceptor.setXmlRpcClient(client);

        TestFunctions proxy = (TestFunctions) ProxyFactory.getProxy(TestFunctions.class,
                interceptor);

        try {
            proxy.multiplyTest("x", 5);
            fail("Should throw exception");
        } catch (XmlRpcRemoteException e) {
            assertEquals(2, e.getFaultCode());
            assertEquals("message", e.getMessage());
        }

        mcClient.verify();
    }

    public static XmlRpcClientRequest anyRequest() {
        org.easymock.EasyMock.anyObject();
        return null;
    }
}
