/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import com.mongodb.DBObject;

import org.sipfoundry.sipxconfig.admin.forwarding.CallSequence;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.common.Closure;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.beans.factory.annotation.Required;

import static org.sipfoundry.sipxconfig.common.DaoUtils.forAllUsersDo;

public class UserForward extends DataSetGenerator {
    public static final String CFWDTIME = "cfwdtm";
    private ForwardingContext m_forwardingContext;

    @Override
    protected DataSet getType() {
        return DataSet.USER_FORWARD;
    }

    @Required
    public void setForwardingContext(ForwardingContext forwardingContext) {
        m_forwardingContext = forwardingContext;
    }

    public void generate(Replicable entity) {
        if (entity instanceof CallSequence) {
            CallSequence cs = (CallSequence) entity;
            User user = cs.getUser();
            generateUser(user);
        }
        if (entity instanceof User) {
            generateUser((User) entity);
        }
    }
    
    private void generateUser(User user) {
        DBObject top = findOrCreate(user);
        CallSequence cs = m_forwardingContext.getCallSequenceForUser(user);
        top.put(CFWDTIME, Integer.toString(cs.getCfwdTime()));
        getDbCollection().save(top);        
    }

    @Override
    public void generate() {
        Closure<User> closure = new Closure<User>() {
            @Override
            public void execute(User user) {
                generate(user);
            }
        };
        forAllUsersDo(getCoreContext(), closure);
    }

}
