/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrest;

import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.Resource;

public class RestServerDefault extends Resource {
    
    public RestServerDefault(Context context, Request request, Response response) {
       super(context,request,response); 
    }
    
    @Override
    public void handleGet() {
        /*
         * Need to put something here to redirect to the wiki page.
         */
        Response response = getResponse();
        String descriptionPage = RestServer.getServiceFinder().getDescriptions();
        response.setEntity(descriptionPage, MediaType.TEXT_HTML);  
        response.setStatus(Status.SUCCESS_OK);
    }
    

}
