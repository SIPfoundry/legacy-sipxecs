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
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.TapestryContext;

public abstract class SpecialAttendantPanel extends BaseComponent {
    @InjectObject("spring:autoAttendantManager")
    public abstract AutoAttendantManager getAutoAttendantManager();

    @InjectObject("spring:tapestry")
    public abstract TapestryContext getTapestry();

    public abstract AutoAttendant getAutoAttendant();

    public abstract void setAutoAttendant(AutoAttendant aa);

    public abstract Mode getMode();

    public abstract void setMode(Mode mode);

    public abstract IPropertySelectionModel getAutoAttendants();

    public abstract void setAutoAttendants(IPropertySelectionModel model);

    public void onApply() {
        boolean enable = Mode.SPECIAL.equals(getMode());
        getAutoAttendantManager().selectSpecial(getAutoAttendant());
        getAutoAttendantManager().setSpecialMode(enable);
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
        if (getAutoAttendants() == null) {
            setAutoAttendants(createAutoAttendantsModel());
            if (getAutoAttendantManager().getSpecialMode()) {
                setMode(Mode.SPECIAL);
            } else {
                setMode(Mode.NORMAL);
            }
            setAutoAttendant(getAutoAttendantManager().getSelectedSpecialAttendant());
        }
        super.renderComponent(writer, cycle);
    }

    private IPropertySelectionModel createAutoAttendantsModel() {
        ObjectSelectionModel model = new ObjectSelectionModel();
        model.setCollection(getAutoAttendantManager().getAutoAttendants());
        model.setLabelExpression("name");
        return getTapestry().instructUserToSelect(model, getMessages());
    }

    public static final class Mode extends Enum {
        public static final Mode NORMAL = new Mode("normal");
        public static final Mode SPECIAL = new Mode("special");

        private Mode(String name) {
            super(name);
        }
    }
}
