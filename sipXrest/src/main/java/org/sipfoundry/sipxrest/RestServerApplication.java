package org.sipfoundry.sipxrest;

import java.io.File;
import java.net.URI;
import java.net.URL;
import java.util.Collection;
import java.util.List;
import org.apache.log4j.Logger;
import org.restlet.Application;
import org.restlet.Context;
import org.restlet.Restlet;
import org.restlet.Route;
import org.restlet.Router;

public class RestServerApplication extends Application {
    private static Logger logger = Logger.getLogger(RestServerApplication.class);

    private Collection<Plugin> plugins;

    public RestServerApplication() throws Exception {
        super();
        try {
            logger.debug("RestServerApplication: create");
            RestServiceFinder restServiceFinder = RestServer.getServiceFinder();
            restServiceFinder.search(System.getProperty("plugin.dir"));
            plugins = restServiceFinder.getPluginCollection();
            logger.debug("Number of Plugins found = " + plugins.size());
        } catch (Exception ex) {
            logger.error("Exception initializing", ex);
        }
    }

    @Override
    public Restlet createRoot() {
        logger.debug("createRoot");
        Context context = getContext();
        Router router = new Router(context);
        try {
            for (Plugin restService : plugins) {
                DigestAuthenticationFilter filter = new DigestAuthenticationFilter(restService);
                restService.attachContext(filter, context, router);
                
            }
        } catch (Exception e) {
            logger.debug("Exception thrown in search: " + e);
        }
     
        router.attachDefault(RestServerDefault.class);
        return router;
    }

}
