/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
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
     * Get the special user name if this service runs under such 
     * a special user ID. Override this method if this service will
     * run under such a special user ID ( for example ~id~~xxx ).
     */
    public String getSpecialUserName() {
        return null;
    }
    /**
     * Get the clear text password of the agent if it is not
     * included in validusers.xml ( this would be the case for
     * special users ~~id~xxx. Only relevant if you are running
     * a SIP service under such a special user.
     * Override this method if your agent is such a user.
     * 
     */
    public String getSpecialUsersClearTextSipPassword() {
        return null;
    }
    /**
     * Return the identity of the agent that is making the request.
     * This is an entry in the validusers.xml database or it can be
     * a special user ID such as ~~id~watcher which is not recorded
     * in the special user database.
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
