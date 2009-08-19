package org.sipfoundry.callcontroller;

import org.apache.log4j.Logger;
import org.restlet.Application;
import org.restlet.Context;
import org.restlet.Restlet;
import org.restlet.Route;
import org.restlet.Router;

public class CallControllerApplication extends Application {
    private static Logger logger = Logger.getLogger(CallControllerApplication.class);
    
    public CallControllerApplication() {
        super();
        logger.debug("CallControllerApplication: create");
    }
	/** 
	 * Creates a root Restlet that will receive all incoming calls. 
	 */  
	@Override  
	public Restlet createRoot() {  
	   logger.debug("createRoot");
	   Context context = getContext();
	   Router router = new Router(context); 
	   CallControllerFilter filter = new CallControllerFilter();
	   filter.setNext(new CallControllerRestlet(context));
	   router.attachDefault(CallControllerDefault.class);
	   Route route = router.attach("/callcontroller/{callingParty}/{calledParty}", filter); 
	   route.extractQuery(CallControllerParams.METHOD, CallControllerParams.METHOD, true);
	   route.extractQuery(CallControllerParams.AGENT,CallControllerParams.AGENT,true);
	   route.extractQuery(CallControllerParams.FORWARDING_ALLOWED, CallControllerParams.FORWARDING_ALLOWED, true);
	   route.extractQuery(CallControllerParams.PIN, CallControllerParams.PIN, true);
	   route.extractQuery(CallControllerParams.SUBJECT, CallControllerParams.SUBJECT, true);
	   return router;
	}

}
