/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package com.workingmouse.webservice.axis;

import javax.xml.rpc.ServiceException;

import org.apache.axis.AxisEngine;
import org.apache.axis.AxisFault;
import org.apache.axis.utils.Messages;
import org.springframework.web.context.WebApplicationContext;

/**
 * Utility class used by the {@link com.workingmouse.webservice.axis.SpringBeanMsgProvider}
 * and the {@link com.workingmouse.webservice.axis.SpringBeanRPCProvider} to retrieve
 * Spring-managed beans from a Spring WebApplicationContext.
 *
 * @author Tom Czarniecki (cThomas AT workingmouse DOT com)
 */
public class SpringBeanProvider {

    /**
     * The server-config.wsdd service parameter used to provide the name of the
     * Spring-managed bean to use as the web service end-point.
     */
    public static final String BEAN_OPTION_NAME = "springBean";

    /** Name of the Application session attribute for the WebApplicationContext */
    private static final String APPLICATION_SESSION_ATTRIBUTE = SpringBeanProvider.class.getName()
            + ".APPLICATION_SESSION_ATTRIBUTE";

    /**
     * Return a bean bound with the given beanName from the WebApplicationContext.
     */
    public Object getBean(AxisEngine engine, String beanName) throws Exception {
        return getWebApplicationContext(engine).getBean(beanName);
    }

    /**
     * Return the class of a bean bound with the given beanName in the WebApplicationContext.
     */
    public Class getBeanClass(AxisEngine engine, String beanName) throws AxisFault {
        try {
            Object bean = getBean(engine, beanName);
            return bean.getClass();

        } catch (Exception e) {
            throw new AxisFault(Messages.getMessage("noClassForService00", beanName), e);
        }
    }

    public static WebApplicationContext getWebApplicationContext(AxisEngine engine) throws ServiceException {
        WebApplicationContext context = (WebApplicationContext) engine.getApplicationSession().get(APPLICATION_SESSION_ATTRIBUTE);
        if (context == null) {
            throw new ServiceException(
                "Cannot find SpringAxisServlet's WebApplicationContext"
                        + " as ServletContext attribute ["
                        + APPLICATION_SESSION_ATTRIBUTE
                        + "]");
        }

        return context;
    }

    public static void setWebApplicationContext(AxisEngine engine, WebApplicationContext context) {
        engine.getApplicationSession().set(APPLICATION_SESSION_ATTRIBUTE, context);
    }
}