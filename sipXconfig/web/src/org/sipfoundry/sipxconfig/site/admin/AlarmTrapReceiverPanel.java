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

import java.util.ArrayList;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmTrapReceiver;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class AlarmTrapReceiverPanel extends BaseComponent {

    @Parameter(required = true)
    public abstract String getLabel();

    @Parameter
    public abstract List<AlarmTrapReceiver> getSnmpAddresses();

    public abstract void setSnmpAddresses(List<AlarmTrapReceiver> alarmTrapReceiver);

    public abstract List<AlarmTrapReceiver> getAlarmTrapReceivers();

    public abstract void setAlarmTrapReceivers(List<AlarmTrapReceiver> alarmTrapReceivers);

    public abstract int getIndex();

    public abstract boolean getAdd();

    public abstract int getRemoveIndex();

    public abstract void setRemoveIndex(int index);

    public AlarmTrapReceiver getAlarmTrapReceiver() {
        return getAlarmTrapReceivers().get(getIndex());
    }

    public void setAlarmTrapReceiver(AlarmTrapReceiver alarmTrapReceiver) {
        getAlarmTrapReceivers().set(getIndex(), alarmTrapReceiver);
    }

    public int getAlarmTrapReceiversSize() {
        return getAlarmTrapReceivers().size();
    }

    public boolean getAlarmTrapReceiversPresent() {
        if (getAlarmTrapReceiversSize() > 0) {
            return true;
        }
        return false;
    }

    public void setAlarmTrapReceiversSize(int size) {
        List<AlarmTrapReceiver> alarmTrapReceivers = new ArrayList<AlarmTrapReceiver>();
        for (int i = 0; i < size; i++) {
            alarmTrapReceivers.add(new AlarmTrapReceiver());
        }
        setAlarmTrapReceivers(alarmTrapReceivers);
    }

    protected void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        setRemoveIndex(-1);
        if (!TapestryUtils.isRewinding(cycle, this)) {
            if (null != getSnmpAddresses()) {
                setAlarmTrapReceivers(getSnmpAddresses());
            }
        }
    }

    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        super.renderComponent(writer, cycle);
        if (TapestryUtils.isRewinding(cycle, this)) {
            afterRewind(cycle);
        }
    }

    private void afterRewind(IRequestCycle cycle) {
        List<AlarmTrapReceiver> alarmTrapReceivers = getAlarmTrapReceivers();
        if (TapestryUtils.isValid(cycle, this) && getAdd()) {
            if (alarmTrapReceivers != null) {
                alarmTrapReceivers.add(new AlarmTrapReceiver());
            }
        }
        int removeIndex = getRemoveIndex();
        if (removeIndex >= 0) {
            alarmTrapReceivers.remove(removeIndex);
            TapestryUtils.getValidator(this).clearErrors();
        }
        setSnmpAddresses(alarmTrapReceivers);
    }
}
