/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package com.workingmouse.webservice.axis;

import org.apache.axis.AxisFault;
import org.apache.axis.MessageContext;
import org.apache.axis.handlers.soap.SOAPService;
import org.apache.axis.providers.java.RPCProvider;

/**
 * Axis provider for RPC-style services that uses Spring Framework
 * to retrieve service classes and resolve their dependencies.
 * Simply delegates to {@link com.workingmouse.webservice.axis.SpringBeanProvider}.
 * <p>
 * To use this class:<br>
 * 1. Configure {@link com.workingmouse.webservice.axis.SpringAxisServlet} as your axis servlet in web.xml.
 * <pre>
 * &lt;servlet&gt;
 *   &lt;servlet-name&gt;axis&lt;/servlet-name&gt;
 *   &lt;display-name&gt;Apache-Axis Servlet&lt;/display-name&gt;
 *   &lt;servlet-class&gt;com.workingmouse.webservice.axis.SpringAxisServlet&lt;/servlet-class&gt;
 * &lt;/servlet&gt;
 * </pre>
 * 2. Configure your server-config.wsdd service to use this class as the service handler.
 * <pre>
 * &lt;service name=&quot;products.jws&quot; provider=&quot;Handler&quot; style=&quot;document&quot; use=&quot;literal&quot;&gt;
 *   &lt;parameter name=&quot;handlerClass&quot; value=&quot;com.workingmouse.webservice.axis.SpringBeanRPCProvider&quot;/&gt;
 *   &lt;parameter name=&quot;wsdlTargetNamespace&quot; value=&quot;http://www.ioof.com.au/schemas/public&quot;/&gt;
 *   &lt;parameter name=&quot;springBean&quot; value=&quot;productWS&quot;/&gt;
 * &lt;/service&gt;
 * </pre>
 * 3. Configure a Spring-managed bean in axis-servlet.xml that will act as the web service end point.
 * <pre>
 * &lt;bean id=&quot;productWS&quot; class=&quot;com.workingmouse.webservice.ProductWebServices&quot;&gt;
 *   &lt;property name=&quot;productServices&quot;&gt;&lt;ref bean=&quot;productServices&quot;/&gt;&lt;/property&gt;
 * &lt;/bean&gt;
 * </pre>
 *
 * @author Tom Czarniecki (cThomas AT workingmouse DOT com)
 *
 * @see com.workingmouse.webservice.axis.SpringAxisServlet
 * @see com.workingmouse.webservice.axis.SpringBeanProvider
 */
public class SpringBeanRPCProvider extends RPCProvider {

    private final SpringBeanProvider provider = new SpringBeanProvider();

    /**
     * @see org.apache.axis.providers.java.JavaProvider#makeNewServiceObject(org.apache.axis.MessageContext, java.lang.String)
     */
    protected Object makeNewServiceObject(MessageContext msgContext, String clsName)
        throws Exception {

        return provider.getBean(msgContext.getAxisEngine(), clsName);
    }

    /**
     * @see org.apache.axis.providers.java.JavaProvider#getServiceClass(java.lang.String, org.apache.axis.handlers.soap.SOAPService, org.apache.axis.MessageContext)
     */
    protected Class getServiceClass(String clsName, SOAPService service, MessageContext msgContext)
        throws AxisFault {

        return provider.getBeanClass(service.getEngine(), clsName);
    }

    /**
     * @see org.apache.axis.providers.java.JavaProvider#getServiceClassNameOptionName()
     */
    protected String getServiceClassNameOptionName() {
        return SpringBeanProvider.BEAN_OPTION_NAME;
    }
}