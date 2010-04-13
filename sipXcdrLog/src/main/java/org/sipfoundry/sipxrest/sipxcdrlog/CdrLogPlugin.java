/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrest.cdrlog;


import org.restlet.Context;
import org.restlet.Filter;
import org.restlet.Route;
import org.restlet.Router;
import org.restlet.data.Request;
import org.sipfoundry.sipxrest.Plugin;
import org.sipfoundry.sipxrest.cdrlog.CdrLogRestlet;

public class CdrLogPlugin extends Plugin {

    @Override
    public void attachContext(Filter filter, Context context, Router router) {
       filter.setNext(new CdrLogRestlet());
       Route cdrRoute = router.attach(this.getMetaInf().getUriPrefix() + "/{user}",filter);
       cdrRoute.extractQuery(CdrLogParams.LIMIT, CdrLogParams.LIMIT, true);
       cdrRoute.extractQuery(CdrLogParams.FROMDATE, CdrLogParams.FROMDATE, true);
    }

    @Override
    public String getAgent(Request request) {
        return (String) request.getAttributes().get(CdrLogParams.USER);
    }
 
}

