/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.rest;

import org.restlet.Application;
import org.restlet.Context;
import org.restlet.Restlet;
import org.restlet.Router;
import org.restlet.ext.spring.SpringContext;
import org.springframework.beans.factory.xml.XmlBeanDefinitionReader;
import org.springframework.core.io.ClassPathResource;

public class RestApplication extends Application {

    public RestApplication(Context context) {
        super(context);
    }

    @Override
    public Restlet createRoot() {
        Router router = new Router(getContext());
        SpringContext springContext = new SpringContext(getContext());
        XmlBeanDefinitionReader xmlReader = new XmlBeanDefinitionReader(springContext);
        xmlReader.loadBeanDefinitions(new ClassPathResource("applicationContext.xml"));
        springContext.refresh();
        RestManager manager = (RestManager) springContext.getBean("manager");
        manager.init(router);

        return router;
    }
}
