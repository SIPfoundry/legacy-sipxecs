package org.sipfoundry.restServer;

import org.restlet.Context;
import org.restlet.Router;

public interface IPlugin {
    String getName();

    void performAction(Context context, Router router);
}
