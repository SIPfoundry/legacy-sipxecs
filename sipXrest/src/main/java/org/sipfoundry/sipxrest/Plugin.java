package org.sipfoundry.sipxrest;

import org.restlet.Context;
import org.restlet.Filter;
import org.restlet.Router;
import org.restlet.data.Request;

public abstract class Plugin {
    private MetaInf metaInf;
    
    protected void setMetaInf(MetaInf metaInf) {
        this.metaInf = metaInf;
    }
    public MetaInf getMetaInf() {
        return this.metaInf;
        
    }
    /**
     * Return the identity of the agent that is making the request.
     * This is an entry in the validusers.xml database.
     * @param request
     * @return
     */
    public abstract String getAgent(Request request);
    /**
     * Attach the filter and context to the router and route the request
     * to the actual restlet. The Filter is the standard security policy.
     * @param filter
     * @param context
     * @param router
     */
    public abstract void attachContext(Filter filter, Context context, Router router);
}
