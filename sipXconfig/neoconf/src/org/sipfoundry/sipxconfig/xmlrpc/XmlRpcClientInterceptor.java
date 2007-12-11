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

import java.lang.reflect.Method;
import java.net.MalformedURLException;

import org.aopalliance.intercept.MethodInterceptor;
import org.aopalliance.intercept.MethodInvocation;
import org.apache.xmlrpc.XmlRpcClient;
import org.apache.xmlrpc.XmlRpcClientRequest;
import org.apache.xmlrpc.XmlRpcException;
import org.springframework.beans.factory.InitializingBean;
import org.springframework.remoting.support.UrlBasedRemoteAccessor;

public class XmlRpcClientInterceptor extends UrlBasedRemoteAccessor implements MethodInterceptor,
        InitializingBean {
    private XmlRpcClient m_xmlRpcClient;

    private String m_methodNamePrefix;

    /**
     * Intercepts method call and executes XML/RPC call instead.
     * 
     * The exceptions handling is a bit unusual here, but this is a reflection of how
     * XmlRpcClient.execute is now coded. When there it encounters server fault it returns the
     * exception instead of throwing it. Proxied interface is trying to cast returned exception to
     * whatever is the return type of the proxied method, which more often than not results in
     * ClassCastException. That's why we checking if return type is XmlRpcException.
     * 
     * The other interesting aspect is that like most Spring remote proxies we are translating
     * checked exceptions to RuntimeExceptions, giving client a chance to handle them but not
     * forcing the proxied interface to define them. The constructor of the XmlRpcRemoteException
     * effectively performs that translation.
     * 
     */
    public Object invoke(MethodInvocation invocation) throws Throwable {
        XmlRpcClientRequest request = new Request(invocation, m_methodNamePrefix);

        try {
            Object result = m_xmlRpcClient.execute(request);
            // strangely execute returns exceptions, instead of throwing them
            if (result instanceof XmlRpcException) {
                // let catch block translate it
                throw (XmlRpcException) result;
            }
            return result;
        } catch (XmlRpcException e) {
            // in cases execute throws exception - we still need to translate
            throw new XmlRpcRemoteException(e);
        } catch (RuntimeException e) {
            // do not repackage RuntimeExceptions
            throw e;
        } catch (Exception e) {
            // repackage only checked exceptions
            throw new XmlRpcRemoteException(e);
        }
    }

    public void afterPropertiesSet() {
        if (getServiceInterface() == null) {
            throw new IllegalArgumentException("serviceInterface is required");
        }
        if (getServiceUrl() == null) {
            throw new IllegalArgumentException("serviceUrl is required");
        }
        try {
            m_xmlRpcClient = new XmlRpcClient(getServiceUrl());
        } catch (MalformedURLException e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * Mostly for testing - one can inject other XmlRpcClient implementations
     * 
     * @param xmlRpcClient client that would be used to make remote calls
     */
    public void setXmlRpcClient(XmlRpcClient xmlRpcClient) {
        m_xmlRpcClient = xmlRpcClient;
    }

    public void setMethodNamePrefix(String methodNamePrefix) {
        m_methodNamePrefix = methodNamePrefix;
    }

    static class Request implements XmlRpcClientRequest {
        private Method m_method;

        private Object[] m_args;

        private String m_methodNamePrefix;

        public Request(MethodInvocation invocation, String methodNamePrefix) {
            m_method = invocation.getMethod();
            m_args = invocation.getArguments();
            m_methodNamePrefix = methodNamePrefix;
        }

        public String getMethodName() {
            if (m_methodNamePrefix == null) {
                return m_method.getName();
            }
            return m_methodNamePrefix + m_method.getName();
        }

        public int getParameterCount() {
            return m_args.length;
        }

        public Object getParameter(int index) {
            return m_args[index];
        }
    }
}
