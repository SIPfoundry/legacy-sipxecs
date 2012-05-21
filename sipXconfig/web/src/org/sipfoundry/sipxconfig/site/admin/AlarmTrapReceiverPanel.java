/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.sipfoundry.sipxconfig.alarm.AlarmServerManager;
import org.sipfoundry.sipxconfig.alarm.AlarmTrapReceiver;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class AlarmTrapReceiverPanel extends BaseComponent {

    @InjectObject("spring:alarmServerManager")
    public abstract AlarmServerManager getAlarmServerManager();

    @Parameter
    public abstract SipxValidationDelegate getValidator();

    public abstract List<AlarmTrapReceiver> getAlarmTrapReceivers();

    public abstract void setAlarmTrapReceivers(List<AlarmTrapReceiver> alarmTrapReceivers);

    public abstract int getIndex();

    public abstract Integer getRemove();

    public abstract void setRemove(Integer index);

    public abstract void setAlarmTrapReceiver(AlarmTrapReceiver alarmTrapReceiver);

    public abstract AlarmTrapReceiver getAlarmTrapReceiver();

    public abstract Integer getAlarmTrapReceiversSize();

    public abstract void setAlarmTrapReceiversSize(Integer size);

    public abstract Boolean getAdd();

    public abstract void setAdd(Boolean b);

    public boolean getAlarmTrapReceiversPresent() {
        return getAlarmTrapReceivers().size() > 0;
    }

    protected void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        initializeData();
    }

    private void initializeData() {
        List<AlarmTrapReceiver> receivers = getAlarmTrapReceivers();
        if (receivers == null) {
            receivers = getAlarmServerManager().getAlarmTrapReceivers();
            setAlarmTrapReceivers(receivers);
        }

        if (getAlarmTrapReceiversSize() == null) {
            setAlarmTrapReceiversSize(receivers.size());
        }

        if (Boolean.TRUE.equals(getAdd())) {
            setAlarmTrapReceiversSize(getAlarmTrapReceiversSize() + 1);
            setAdd(null);
        }

        if (receivers.size() < getAlarmTrapReceiversSize()) {
            for (int i = receivers.size(); i < getAlarmTrapReceiversSize(); i++) {
                receivers.add(new AlarmTrapReceiver());
            }
        }

        if (getRemove() != null) {
            AlarmTrapReceiver gone = receivers.remove(getRemove().intValue());
            setRemove(null);
            setAlarmTrapReceiversSize(getAlarmTrapReceiversSize() - 1);
            if (gone.getId() > 0) {
                getAlarmServerManager().deleteAlarmTrapReceiver(gone);
                TapestryUtils.recordSuccess(this, getMessages().getMessage("msg.actionSuccess"));
            }
        }
    }

    public IPrimaryKeyConverter getTrapConverter() {
        initializeData();
        final List<AlarmTrapReceiver> receivers = getAlarmTrapReceivers();
        return new IPrimaryKeyConverter() {
            public Object getPrimaryKey(Object value) {
                return receivers.lastIndexOf(value);
            }
            public Object getValue(Object primaryKey) {
                return receivers.get((Integer) primaryKey);
            }
        };
    }

    public void save() {
        if (TapestryUtils.isValid(this)) {
            getAlarmServerManager().saveAlarmTrapReceivers(getAlarmTrapReceivers());
        }
    }
}
