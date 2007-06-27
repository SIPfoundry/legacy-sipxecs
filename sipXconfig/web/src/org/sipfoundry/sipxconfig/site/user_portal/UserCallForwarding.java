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

import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.forwarding.AbstractSchedule;
import org.sipfoundry.sipxconfig.admin.forwarding.CallSequence;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.admin.forwarding.Ring;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.permission.PermissionName;

/**
 * UserCallForwarding
 */
public abstract class UserCallForwarding extends UserBasePage implements PageBeginRenderListener {
    public static final String PAGE = "UserCallForwarding";
    private static final String ACTION_ADD = "add";

    @InjectObject(value = "spring:forwardingContext")
    public abstract ForwardingContext getForwardingContext();

    @Persist
    public abstract List<Ring> getRings();

    public abstract void setRings(List<Ring> rings);

    public abstract String getAction();

    public abstract Ring getRing();

    public abstract int getIndex();

    public abstract List getAvailableSchedules();

    public abstract void setAvailableSchedules(List schedules);

    public void pageBeginRender(PageEvent event) {
        if (getRings() != null) {
            refreshAvailableSchedules();
            return;
        }

        super.pageBeginRender(event);

        refreshAvailableSchedules();

        List rings = createDetachedRingList(getCallSequence());
        setRings(rings);
    }

    private void refreshAvailableSchedules() {
        ForwardingContext forwardingContext = getForwardingContext();
        forwardingContext.loadAlwaysSchedule();
        List<AbstractSchedule> availableSchedules = getSchedules();
        setAvailableSchedules(availableSchedules);
    }

    /**
     * Create list of rings that is going to be stored in session.
     * 
     * The list is a clone of the list kept by current call sequence, ring objects do not have
     * valid ids and their call sequence field is set to null.
     */
    private List<Ring> createDetachedRingList(CallSequence callSequence) {
        List<Ring> rings = callSequence.getRings();
        List<Ring> list = new ArrayList<Ring>();
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
        Integer userId = getUserId();
        return forwardingContext.getCallSequenceForUserId(userId);
    }

    private List<AbstractSchedule> getSchedules() {
        ForwardingContext forwardingContext = getForwardingContext();
        Integer userId = getUserId();
        return forwardingContext.getSchedulesForUserIdIncludingAlways(userId);
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
        getForwardingContext().saveCallSequence(callSequence);
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

    /**
     * Users who do not have external call permission should be warned that call forwarding will
     * not work.
     */
    public boolean getRenderForwardWarning() {
        return !getUser().hasPermission(PermissionName.FORWARD_CALLS_EXTERNAL);
    }

    public String getForwardWarning() {
        return getMessages().format("warn.external", getUser().getUserName());
    }
}
