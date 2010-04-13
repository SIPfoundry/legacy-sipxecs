/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.line;

import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.InjectPage;
import org.sipfoundry.sipxconfig.site.common.BeanNavigation;

public abstract class LineNavigation extends BeanNavigation {

    @InjectPage(value = LineSettings.PAGE)
    public abstract LineSettings getLineSettingsPage();

    @InjectPage(value = EditLine.PAGE)
    public abstract EditLine getEditLinePage();

    public boolean isIdentificationTabActive() {
        return EditLine.PAGE.equals(getPage().getPageName());
    }

    public IPage editLine(Integer lineId) {
        EditLine page = getEditLinePage();
        page.setLineId(lineId);
        return page;
    }

    public IPage editSettings(Integer lineId, String section) {
        LineSettings page = getLineSettingsPage();
        page.setLineId(lineId);
        page.setParentSettingName(section);
        return page;
    }
}
