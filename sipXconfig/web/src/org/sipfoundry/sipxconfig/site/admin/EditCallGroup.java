/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.apache.tapestry.AbstractPage;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.PrimaryKeySource;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.user.SelectUsers;
import org.sipfoundry.sipxconfig.site.user.SelectUsersCallback;

public abstract class EditCallGroup extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "admin/EditCallGroup";

    public abstract CallGroupContext getCallGroupContext();

    public abstract Integer getCallGroupId();

    public abstract void setCallGroupId(Integer id);

    public abstract CallGroup getCallGroup();

    public abstract void setCallGroup(CallGroup callGroup);

    public abstract Collection getNewUsersIds();

    public abstract void setNewUsersIds(Collection participantsIds);

    public void pageBeginRender(PageEvent event_) {
        addNewUsers();
        CallGroup callGroup = getCallGroup();
        if (null != callGroup) {
            return;
        }
        Integer id = getCallGroupId();
        if (null != id) {
            CallGroupContext context = getCallGroupContext();
            callGroup = context.loadCallGroup(id);
        } else {
            callGroup = new CallGroup();
        }
        setCallGroup(callGroup);
        if (null == getCallback()) {
            setReturnPage(ListCallGroups.PAGE);
        }
    }

    protected void addNewUsers() {
        Integer id = getCallGroupId();
        Collection ids = getNewUsersIds();
        if (id != null && ids != null) {
            getCallGroupContext().addUsersToCallGroup(id, ids);
        }
        setNewUsersIds(null);
    }

    /**
     * Called when any of the submit components on the form is activated.
     *
     * Usually submit components are setting properties. formSubmit will first check if the form
     * is valid, then it will call all the "action" listeners. Only one of the listeners (the one
     * that recognizes the property that is set) will actually do something. This is a strange
     * consequence of the fact that Tapestry listeners are pretty much useless because they are
     * called while the form is still rewinding and not all changes are committed to beans.
     *
     * @param cycle current request cycle
     */
    public IPage formSubmit(IRequestCycle cycle) {
        if (!isValid()) {
            return null;
        }
        UserRingTable ringTable = getUserRingTable();
        if (delete(ringTable) || move(ringTable)) {
            saveValid();
        }
        return addRow(cycle, ringTable);
    }

    public void commit() {
        if (!isValid()) {
            return;
        }
        saveValid();
    }

    /**
     * Saves current call group and displays add ring page.
     *
     * @param cycle current request cycle
     * @param ringTable component with table of rings
     */
    private IPage addRow(IRequestCycle cycle, UserRingTable ringTable) {
        if (!ringTable.getAddRow()) {
            return null;
        }
        saveValid();
        SelectUsers page = (SelectUsers) cycle.getPage(SelectUsers.PAGE);
        page.setTitle(getMessages().getMessage("title.selectRings"));
        page.setPrompt(getMessages().getMessage("prompt.selectRings"));
        page.setCallback(new SelectRingsCallback(getCallGroupId()));
        return page;
    }

    private boolean isValid() {
        IValidationDelegate delegate = TapestryUtils.getValidator(this);
        return !delegate.getHasErrors();
    }

    private void saveValid() {
        CallGroupContext context = getCallGroupContext();
        CallGroup callGroup = getCallGroup();
        context.storeCallGroup(callGroup);
        Integer id = getCallGroup().getId();
        setCallGroupId(id);
    }

    private boolean delete(UserRingTable ringTable) {
        Collection ids = ringTable.getRowsToDelete();
        if (null == ids) {
            return false;
        }
        CallGroup callGroup = getCallGroup();
        callGroup.removeRings(ids);
        return true;
    }

    private boolean move(UserRingTable ringTable) {
        int step = -1;
        Collection ids = ringTable.getRowsToMoveUp();
        if (null == ids) {
            step = 1;
            ids = ringTable.getRowsToMoveDown();
            if (null == ids) {
                // nothing to do
                return false;
            }
        }
        CallGroup callGroup = getCallGroup();
        callGroup.moveRings(ids, step);
        return true;
    }

    private UserRingTable getUserRingTable() {
        return (UserRingTable) getComponent("ringTable");
    }

    private static class SelectRingsCallback extends SelectUsersCallback {
        private Integer m_callGroupId;

        public SelectRingsCallback(Integer callGroupId) {
            super(PAGE);
            setIdsPropertyName("newUsersIds");
            m_callGroupId = callGroupId;
        }

        protected void beforeActivation(AbstractPage page) {
            EditCallGroup editCallGroup = (EditCallGroup) page;
            editCallGroup.setCallGroupId(m_callGroupId);
        }
    }

    public IPrimaryKeyConverter getIdConverter() {
        return new RingConverter(getCallGroup());
    }

    public static final class RingConverter implements IPrimaryKeyConverter {
        private CallGroup m_callGroup;

        public RingConverter(CallGroup group) {
            m_callGroup = group;
        }

        public Object getPrimaryKey(Object objValue) {
            BeanWithId bean = (BeanWithId) objValue;
            return bean.getPrimaryKey();
        }

        public Object getValue(Object objPrimaryKey) {
            List rings = m_callGroup.getRings();
            for (Iterator i = rings.iterator(); i.hasNext();) {
                PrimaryKeySource bean = (PrimaryKeySource) i.next();
                if (bean.getPrimaryKey().equals(objPrimaryKey)) {
                    return bean;
                }
            }
            return null;
        }
    }
}
