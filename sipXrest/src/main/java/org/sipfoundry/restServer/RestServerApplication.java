package org.sipfoundry.restServer;

import java.io.File;
import java.net.URI;
import java.net.URL;
import java.util.List;
import org.apache.log4j.Logger;
import org.restlet.Application;
import org.restlet.Context;
import org.restlet.Restlet;
import org.restlet.Route;
import org.restlet.Router;

public class RestServerApplication extends Application {
    private static Logger logger = Logger.getLogger(RestServerApplication.class);
    
    public RestServerApplication() {
        super();
        logger.debug("RestServerApplication: create");
    }
	/** 
	 * Creates a root Restlet that will receive all incoming calls. 
	 */  
	@Override  
	public Restlet createRoot() {  
	   logger.debug("createRoot");
	   Context context = getContext();
	   Router router = new Router(context); 

           List<IPlugin> plugins;
           RestServiceFinder restPlugins = new RestServiceFinder();
           try {
               URI myURI = new URI(RestServerApplication.class.getResource("RestServerApplication.class").toString());
               File dir2 = new File(myURI.toString());
               String dirString = dir2.toString();
               dirString = dirString.substring(0, dirString.lastIndexOf('!'));
               dirString = dirString.substring(dirString.indexOf('/'),dirString.lastIndexOf('/'));

               restPlugins.search(dirString + "/restplugins");
               plugins = restPlugins.getPluginCollection();
               logger.debug("Number of Plugins found = " + plugins.size());
               for (IPlugin restService : plugins) {
                   logger.debug("Service " + restService.getName() + " found and loaded");
                   restService.performAction(context, router);
               }
           }
           catch (Exception e) {
              logger.debug("Exception thrown in search: " + e);
           }

	   router.attachDefault(RestServerDefault.class);
	   return router;
	}

}
