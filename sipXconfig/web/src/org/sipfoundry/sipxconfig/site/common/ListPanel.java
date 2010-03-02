/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.common;

import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.form.validator.Validator;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class ListPanel extends BaseComponent {
    @Parameter(required = true)
    public abstract List getSource();

    @Parameter(required = true)
    public abstract String getLabel();

    @Parameter(required = true)
    public abstract String getItemDisplayName();

    @Parameter(defaultValue = "message:button.add")
    public abstract String getAddLinkLabel();

    @Parameter
    public abstract List<Validator> getValidators();

    public abstract int getIndex();

    public abstract boolean getAdd();

    public abstract int getRemoveIndex();

    public abstract void setRemoveIndex(int index);

    public Object getValue() {
        return getSource().get(getIndex());
    }

    public void setValue(Object value) {
        getSource().set(getIndex(), value);
    }

    public int getSize() {
        return getSource().size();
    }

    public void setSize(int size) {
        List source = getSource();
        while (size < source.size()) {
            source.remove(source.size() - 1);
        }
        while (size > source.size()) {
            source.add(null);
        }
    }

    protected void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        setRemoveIndex(-1);
    }

    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        super.renderComponent(writer, cycle);
        afterRewind(cycle);
    }

    private void afterRewind(IRequestCycle cycle) {
        List source = getSource();
        int removeIndex = getRemoveIndex();
        if (removeIndex >= 0) {
            source.remove(removeIndex);
            TapestryUtils.getValidator(this).clearErrors();
        } else if (TapestryUtils.isValid(cycle, this) && getAdd()) {
            source.add(null);
        }
    }
}
