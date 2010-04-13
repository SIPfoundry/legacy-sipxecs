/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrest.testa;



import org.restlet.Context;
import org.restlet.Filter;
import org.restlet.Route;
import org.restlet.Router;
import org.restlet.data.Request;
import org.sipfoundry.sipxrest.Plugin;
import org.sipfoundry.sipxrest.testa.RestletA;

public class PluginA extends Plugin {


    @Override
    public void attachContext(Filter filter, Context context, Router router) {
       filter.setNext(new RestletA());
       Route route = router.attach(getMetaInf().getUriPrefix() + "/{param}",filter);
       route.extractQuery("agent", "agent", true);
    }

    @Override
    public String getAgent(Request request) {
        return (String) request.getAttributes().get("agent");
    }
}
