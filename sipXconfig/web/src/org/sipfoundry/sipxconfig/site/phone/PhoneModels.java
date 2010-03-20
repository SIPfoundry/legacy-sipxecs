/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.phone;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.site.setting.EditGroup;

public abstract class PhoneModels extends SipxBasePage implements PageBeginRenderListener {

    public static final String PAGE = "phone/PhoneModels";

    public abstract void setGroupId(Integer id);

    public abstract Integer getGroupId();

    public abstract void setGroup(Group group);

    public abstract SettingDao getSettingContext();

    public abstract ModelSource<PhoneModel> getPhoneModelSource();

    public IPage editGroup(IRequestCycle cycle) {
        EditGroup page = (EditGroup) cycle.getPage(EditGroup.PAGE);
        page.editGroup(getGroupId(), PAGE);
        return page;
    }

    public IPage editPhoneDefaults(IRequestCycle cycle, String modelId) {
        PhoneModel model = getPhoneModelSource().getModel(modelId);
        EditPhoneDefaults page = (EditPhoneDefaults) cycle.getPage(EditPhoneDefaults.PAGE);
        page.editPhoneSettings(model, getGroupId());
        return page;
    }

    public void pageBeginRender(PageEvent event_) {
        Group g = getSettingContext().loadGroup(getGroupId());
        setGroup(g);
    }
}
