/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.xmlrpc;

import java.io.IOException;
import java.lang.reflect.Method;
import java.net.MalformedURLException;
import java.util.Arrays;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.FutureTask;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import static java.lang.String.format;

import org.aopalliance.intercept.MethodInterceptor;
import org.aopalliance.intercept.MethodInvocation;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.xmlrpc.XmlRpcClient;
import org.apache.xmlrpc.XmlRpcClientRequest;
import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.secure.SecureXmlRpcClient;
import org.sipfoundry.sipxconfig.common.SipxCollectionUtils;
import org.springframework.beans.factory.BeanInitializationException;
import org.springframework.remoting.support.UrlBasedRemoteAccessor;

public class XmlRpcClientInterceptor extends UrlBasedRemoteAccessor implements MethodInterceptor {
    private static final Log LOG = LogFactory.getLog(XmlRpcClientInterceptor.class);

    private XmlRpcClient m_xmlRpcClient;

    private boolean m_secure;

    private XmlRpcMarshaller m_marshaller = new DefaultMarshaller(null);

    // was 5000 (5s) but around 8K users on fast hard, was experiencing timeouts. Bumped to 1m
    private long m_timeout = 60000;

    private final ExecutorService m_service = Executors.newSingleThreadExecutor();

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
        final Request request = new Request(invocation, m_marshaller);
        if (LOG.isInfoEnabled()) {
            String msg = format("XML/RPC %s on %s", request.dump(LOG.isDebugEnabled()), getServiceUrl());
            LOG.info(msg);
        }

        try {
            return executeWithTimeout(request);
        } catch (RuntimeException e) {
            LOG.error("Runtime error in XML/RPC call", e);
            // do not repackage RuntimeExceptions
            throw e;
        } catch (Exception e) {
            LOG.error("Exception in XML/RPC call", e);
            // repackage only checked exceptions
            throw new XmlRpcRemoteException(e);
        }
    }

    @Override
    public void afterPropertiesSet() {
        super.afterPropertiesSet();
        if (getServiceInterface() == null) {
            throw new IllegalArgumentException("serviceInterface is required");
        }
        try {
            if (m_secure) {
                m_xmlRpcClient = new SecureXmlRpcClient(getServiceUrl());
            } else {
                m_xmlRpcClient = new XmlRpcClient(getServiceUrl());
            }
        } catch (MalformedURLException e) {
            throw new BeanInitializationException("Cannot create XML/RPC proxy.", e);
        }
    }

    /**
     * Temporary workaround for the problem with timeout (or lack thereof) in the XML/RPC library
     * transport.
     *
     * We call the XML/RPC in the background thread and interrupt it if it takes too long. We
     * attempt to handle exceptions related to the threading issues (And timeout) only. Everything
     * else is handled by the invoke method.
     *
     * @param request XML/RPC request
     * @return result of XML/RPC call
     */
    private Object executeWithTimeout(final XmlRpcClientRequest request) throws Throwable {
        Callable execute = new Callable() {
            public Object call() throws IOException {
                try {
                    Object result = m_xmlRpcClient.execute(request);
                    // strangely execute returns exceptions, instead of throwing them
                    if (result instanceof XmlRpcException) {
                        // let catch block translate it
                        throw (XmlRpcException) result;
                    }
                    return result;
                } catch (XmlRpcException e) {
                    LOG.error("XML/RPC error: ", e);
                    // in cases execute throws exception - we still need to translate
                    throw new XmlRpcRemoteException(e);
                }
            }
        };
        FutureTask task = new FutureTask(execute);
        m_service.execute(task);
        try {
            return task.get(m_timeout, TimeUnit.MILLISECONDS);
        } catch (InterruptedException e) {
            throw new XmlRpcRemoteException(e);
        } catch (TimeoutException e) {
            LOG.error("Timeout in XML/RPC call");
            throw new XmlRpcRemoteException(e);
        } catch (ExecutionException e) {
            // rethrow cause and let us to repackage it below
            throw e.getCause();
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
        m_marshaller = new DefaultMarshaller(methodNamePrefix);
    }

    public void setSecure(boolean secure) {
        m_secure = secure;
    }

    public void setMarshaller(XmlRpcMarshaller marshaller) {
        m_marshaller = marshaller;
    }

    public void setTimeout(long timeout) {
        m_timeout = timeout;
    }

    static class Request implements XmlRpcClientRequest {
        private final String m_methodName;
        private final Object[] m_args;

        public Request(MethodInvocation invocation, XmlRpcMarshaller marshaller) {
            Method method = invocation.getMethod();
            String name = method.getName();
            m_methodName = marshaller.methodName(name);
            m_args = marshaller.parameters(name, invocation.getArguments());
        }

        public String getMethodName() {
            return m_methodName;
        }

        public int getParameterCount() {
            return m_args.length;
        }

        public Object getParameter(int index) {
            return m_args[index];
        }

        @Override
        public String toString() {
            return dump(true);
        }

        public String dump(boolean full) {
            String params = full ? Arrays.deepToString(m_args) : SipxCollectionUtils.arrayToShortString(m_args, 25);
            return format("%s with %s", m_methodName, params);
        }
    }

    /**
     * Default marshaller adds method name prefix but does not change arguments.
     */
    static class DefaultMarshaller implements XmlRpcMarshaller {
        private final String m_methodNamePrefix;

        public DefaultMarshaller(String methodNamePrefix) {
            m_methodNamePrefix = methodNamePrefix;
        }

        public String methodName(String name) {
            if (m_methodNamePrefix == null) {
                return name;
            }
            return m_methodNamePrefix + name;
        }

        public Object[] parameters(String name, Object... args) {
            return args;
        }
    }
}
