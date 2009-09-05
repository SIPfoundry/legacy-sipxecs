package org.sipfoundry.sipxrest.testa;



import org.apache.log4j.Logger;
import org.restlet.Restlet;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;

public class RestletA extends Restlet {
    private static Logger logger = Logger.getLogger(RestletA.class);
    
    public RestletA () {
        
    }
    
   
    @Override
    public void handle(Request request, Response response) {
        System.out.println("RestletA: got a request ");
        System.out.println("Parameter A is " + request.getAttributes().get("param"));
        response.setStatus(Status.SUCCESS_OK);
        
    }
  

}
