/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.api;

import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.Set;

import org.apache.commons.collections.CollectionUtils;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.phone.LineInfo;

public class PhoneBuilder extends SimpleBeanBuilder {
    private static final String MODEL_ID_PROP = "modelId";
    private static final String GROUPS_PROP = "groups";
    private static final String LINES_PROP = "lines";
    private static final String DEVICE_VER = "deviceVersion";

    private static final String[] CUSTOM_FIELDS = {
        MODEL_ID_PROP, GROUPS_PROP, LINES_PROP, DEVICE_VER
    };

    public PhoneBuilder() {
        getCustomFields().addAll(Arrays.asList(CUSTOM_FIELDS));
    }

    public void toApiObject(Object apiObject, Object myObject, Set properties) {
        super.toApiObject(apiObject, myObject, properties);
        Phone phone = (Phone) apiObject;
        org.sipfoundry.sipxconfig.phone.Phone otherPhone = (org.sipfoundry.sipxconfig.phone.Phone) myObject;
        if (properties.contains(MODEL_ID_PROP)) {
            phone.setModelId(otherPhone.getModelId());
        }
        if (properties.contains(GROUPS_PROP)) {
            Collection groupNames = CollectionUtils.collect(otherPhone.getGroups(),
                    new NamedObject.ToName());
            phone.setGroups((String[]) groupNames.toArray(new String[groupNames.size()]));
        }
        if (properties.contains(LINES_PROP)) {
            List myLines = otherPhone.getLines();
            if (myLines.size() > 0) {
                Line[] apiLines = (Line[]) ApiBeanUtil.newArray(Line.class, myLines.size());
                for (int i = 0; i < apiLines.length; i++) {
                    org.sipfoundry.sipxconfig.phone.Line myLine = (org.sipfoundry.sipxconfig.phone.Line) myLines
                            .get(i);
                    LineInfo lineInfo = myLine.getLineInfo();
                    apiLines[i].setUserId(lineInfo.getUserId());
                    apiLines[i].setUri(myLine.getUri());
                }
                phone.setLines(apiLines);
            }
        }
        DeviceVersion ver = otherPhone.getDeviceVersion();
        if (ver != null && properties.contains(DEVICE_VER)) {
            phone.setDeviceVersion(ver.getName());
        }
    }

    public void toMyObject(Object myObject, Object apiObject, Set properties) {
        super.toMyObject(myObject, apiObject, properties);
        Phone phone = (Phone) apiObject;
        org.sipfoundry.sipxconfig.phone.Phone otherPhone = (org.sipfoundry.sipxconfig.phone.Phone) myObject;
        String verId = phone.getDeviceVersion();
        if (verId != null) {
            DeviceVersion ver = DeviceVersion.getDeviceVersion(verId);
            if (ver == null) {
                throw new RuntimeException(String.format("Invalid device version id '%s'", verId));
            }
            otherPhone.setDeviceVersion(ver);
        }
    }
}
