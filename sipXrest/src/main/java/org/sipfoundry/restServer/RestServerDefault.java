package org.sipfoundry.restServer;

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
        response.setEntity("Hello World!", MediaType.TEXT_PLAIN);  
        response.setStatus(Status.SUCCESS_OK);
    }
    

}
