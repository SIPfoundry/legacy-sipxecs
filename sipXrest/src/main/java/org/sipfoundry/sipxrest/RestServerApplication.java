/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrest;

import java.io.File;
import java.net.URI;
import java.net.URL;
import java.util.Collection;
import java.util.List;
import org.apache.log4j.Logger;
import org.restlet.Application;
import org.restlet.Context;
import org.restlet.Filter;
import org.restlet.Restlet;
import org.restlet.Route;
import org.restlet.Router;

public class RestServerApplication extends Application {
    private static Logger logger = Logger.getLogger(RestServerApplication.class);

    private Collection<Plugin> plugins;

    public RestServerApplication() throws Exception {
       super();
       this.plugins = RestServer.getServiceFinder().getPluginCollection();
    }

    @Override
    public Restlet createRoot() {
        logger.debug("createRoot");
        Context context = getContext();
        Router router = new Router(context);
        try {
            for (Plugin restService : plugins) {
                Filter filter = null;
                if ( restService.getMetaInf().getSecurity().equals(MetaInf.LOCAL_ONLY)) {
                    filter = new LocalOnlyFilter();                 
                } if ( restService.getMetaInf().getRemoteAuthenticationMethod().equals(MetaInf.HTTP_DIGEST)) {
                    filter = new DigestAuthenticationFilter(restService);        
                } else if ( restService.getMetaInf().getRemoteAuthenticationMethod().equals(MetaInf.HTTPS_BASIC)) {
                   filter = new BasicAuthenticationFilter(restService);          
                } else if ( restService.getMetaInf().getRemoteAuthenticationMethod().equals(MetaInf.BASIC_AND_DIGEST)) {
                    filter = new BasicOrDigestAuthenticationFilter(restService);
                } else {
                    logger.error("Unknown remote authentication type -- rejecting the plugin");
                }
                restService.attachContext(filter, context, router);               
            }
        } catch (Exception e) {
            logger.debug("Exception thrown in search: " + e);
        }
     
        router.attachDefault(RestServerDefault.class);
        return router;
    }

}
