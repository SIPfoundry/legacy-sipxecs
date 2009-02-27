/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.dialplan;

import org.apache.commons.lang.enums.Enum;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendantManager;
import org.sipfoundry.sipxconfig.components.LocalizedOptionModelDecorator;

public abstract class SpecialAttendantPanel extends BaseComponent {
    @InjectObject("spring:autoAttendantManager")
    public abstract AutoAttendantManager getAutoAttendantManager();

    public abstract AutoAttendant getAutoAttendant();

    public abstract Mode getMode();

    public abstract void setMode(Mode mode);

    public void onApply() {
        boolean enable = Mode.SPECIAL.equals(getMode());
        getAutoAttendantManager().specialAutoAttendantMode(enable, getAutoAttendant());
    }

    public IPropertySelectionModel getModeModel() {
        LocalizedOptionModelDecorator model = new LocalizedOptionModelDecorator();
        model.setModel((IPropertySelectionModel) getBeans().getBean("modeModel"));
        model.setMessages(getMessages());
        model.setResourcePrefix("mode.");
        return model;
    }

    @Override
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        // use standard mode by default
        if (getMode() == null) {
            setMode(Mode.NORMAL);
        }
        super.renderComponent(writer, cycle);
    }

    public static final class Mode extends Enum {
        public static final Mode NORMAL = new Mode("normal");
        public static final Mode SPECIAL = new Mode("special");

        private Mode(String name) {
            super(name);
        }
    }
}
