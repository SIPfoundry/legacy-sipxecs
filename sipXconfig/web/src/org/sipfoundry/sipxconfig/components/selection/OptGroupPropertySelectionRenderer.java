/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components.selection;

import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.IPropertySelectionRenderer;
import org.apache.tapestry.form.PropertySelection;

public class OptGroupPropertySelectionRenderer implements IPropertySelectionRenderer {
    protected static final String LABEL_ATTR = "label";
    protected static final String DISABLED_ATTR = "disabled";
    protected static final String OPTION_ELEM = "option";
    protected static final String VALUE_ATTR = "value";
    protected static final String SELECTED_ATTR = "selected";

    private boolean m_insideOptGroup;

    protected final boolean isInsideOptGroup() {
        return m_insideOptGroup;
    }

    public final void beginRender(PropertySelection component, IMarkupWriter writer,
            IRequestCycle cycle_) {
        m_insideOptGroup = false;
        writer.begin("select");
        writer.attribute("name", component.getName());
    }

    public final void renderOption(PropertySelection component_, IMarkupWriter writer,
            IRequestCycle cycle_, IPropertySelectionModel model, Object option, int index,
            boolean selected) {
        if (option instanceof OptGroup) {
            renderOptGroup(writer, (OptGroup) option, index);
        } else {
            renderNormalOption(writer, model, index, selected, option == null);
        }
    }

    public final void endRender(PropertySelection component_, IMarkupWriter writer,
            IRequestCycle cycle_) {
        endOptionGroup(writer);
        writer.end(); // <select>
    }

    protected void renderNormalOption(IMarkupWriter writer, IPropertySelectionModel model,
            int index, boolean selected, boolean disabled) {
        writer.begin(OPTION_ELEM);
        writer.attribute(VALUE_ATTR, model.getValue(index));
        if (selected) {
            writer.attribute(SELECTED_ATTR, true);
        }
        if (disabled) {
            writer.attribute(DISABLED_ATTR, true);
        }
        String label = model.getLabel(index);
        writer.attribute(LABEL_ATTR, label);
        writer.print(label);
        writer.end();
        writer.println();
    }

    protected void renderOptGroup(IMarkupWriter writer, OptGroup optGroup, int index) {
        endOptionGroup(writer);
        writer.begin("optgroup");
        writer.attribute(LABEL_ATTR, optGroup.getLabel(optGroup, index));
        writer.println();
        m_insideOptGroup = true;
    }

    private void endOptionGroup(IMarkupWriter writer) {
        if (m_insideOptGroup) {
            writer.end();
            m_insideOptGroup = false;
        }
    }
}
