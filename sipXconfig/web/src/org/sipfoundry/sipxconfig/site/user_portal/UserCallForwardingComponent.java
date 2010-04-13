/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.user_portal;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.forwarding.CallSequence;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.admin.forwarding.Ring;
import org.sipfoundry.sipxconfig.admin.forwarding.Schedule;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.site.UserSession;

public abstract class UserCallForwardingComponent extends BaseComponent implements PageBeginRenderListener {
    private static final String ACTION_ADD = "add";

    @InjectObject(value = "spring:forwardingContext")
    public abstract ForwardingContext getForwardingContext();

    @Persist
    public abstract List<Ring> getRings();

    public abstract void setRings(List<Ring> rings);

    public abstract Integer getUserExpiration();

    public abstract void setUserExpiration(Integer expiration);

    public abstract String getAction();

    public abstract Ring getRing();

    public abstract int getIndex();

    public abstract List getAvailableSchedules();

    public abstract void setAvailableSchedules(List schedules);

    @Parameter(required = true)
    public abstract User getUser();

    @Parameter(required = true)
    public abstract UserSession getUserSession();

    @Persist(value = "client")
    public abstract Integer getCurrentUserId();

    public abstract void setCurrentUserId(Integer userId);

    public void pageBeginRender(PageEvent event) {
        if (getCurrentUserId() == null) {
            setRings(null);
        }
        setCurrentUserId(getUser().getId());

        final CallSequence callSequence = getCallSequence();

        if (getUserExpiration() == null) {
            setUserExpiration(callSequence.getCfwdTime());
        }
        if (getRings() != null) {
            refreshAvailableSchedules();
            return;
        }

        refreshAvailableSchedules();
        List rings = createDetachedRingList(callSequence);
        setRings(rings);
    }

    private void refreshAvailableSchedules() {
        ForwardingContext forwardingContext = getForwardingContext();
        List<Schedule> schedules = forwardingContext.getAllAvailableSchedulesForUser(getUser());
        setAvailableSchedules(schedules);
    }

    /**
     * Create list of rings that is going to be stored in session.
     *
     * The list is a clone of the list kept by current call sequence, ring objects do not have
     * valid ids and their call sequence field is set to null.
     */
    private List createDetachedRingList(CallSequence callSequence) {
        List rings = callSequence.getRings();
        List list = new ArrayList();
        for (Iterator<Ring> i = rings.iterator(); i.hasNext();) {
            BeanWithId ring = i.next();
            Ring dup = (Ring) ring.duplicate();
            dup.setCallSequence(null);
            list.add(dup);
        }
        return list;
    }

    private CallSequence getCallSequence() {
        ForwardingContext forwardingContext = getForwardingContext();
        Integer userId = getUser().getId();
        return forwardingContext.getCallSequenceForUserId(userId);
    }

    public void submit() {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }
        if (ACTION_ADD.equals(getAction())) {
            getRings().add(new Ring());
        }
    }

    public void commit() {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }

        CallSequence callSequence = getCallSequence();
        callSequence.clear();
        callSequence.insertRings(getRings());
        callSequence.setCfwdTime(getUserExpiration());
        getForwardingContext().saveCallSequence(callSequence);
        getForwardingContext().flush();
    }

    public void deleteRing(int position) {
        getRings().remove(position);
    }

    public String getFirstCallMsg() {
        return getMessages().format("msg.first", getUser().getUserName());
    }

    public boolean getHasVoiceMail() {
        return getUser().hasPermission(PermissionName.VOICEMAIL);
    }
}
