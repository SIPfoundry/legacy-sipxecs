/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.site.common;

import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.contrib.form.CheckBoxMultiplePropertySelectionRenderer;
import org.apache.tapestry.contrib.form.MultiplePropertySelection;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.CustomOptionModelDecorator;

public class BetterMultiplePropertySelectionRenderer extends CheckBoxMultiplePropertySelectionRenderer {

    public abstract static class Model extends CustomOptionModelDecorator {
        public abstract String getDescription(String rawLabel);
    }

    @Override
    public void renderOption(MultiplePropertySelection component, IMarkupWriter writer, IRequestCycle cycle,
            IPropertySelectionModel model, Object option, int index, boolean selected) {
        String tr = "tr";
        String td = "td";
        String cls = "class";
        writer.begin(tr);
        writer.begin(td);

        writer.beginEmpty("input");
        writer.attribute("type", "checkbox");
        writer.attribute("name", component.getName());
        writer.attribute("value", model.getValue(index));

        if (component.isDisabled()) {
            String disabled = "disabled".intern();
            writer.attribute(disabled, disabled);
        }

        if (selected) {
            String checked = "checked".intern();
            writer.attribute(checked, checked);
        }

        writer.end(); // <td>
        writer.println();

        writer.begin(td);

        writer.begin("label");
        writer.attribute(cls, "settingLabel");
        writer.print(model.getLabel(index));
        writer.end(); // label
        writer.end(); // <td>
        writer.println();
        writer.end(); // <tr>
        writer.println();

        if (model instanceof Model) {
            Model better = (Model) model;
            String rawLabel = ((Model) model).getRawLabel(index);

            writer.begin(tr);
            writer.beginEmpty(td);
            writer.begin(td);
            writer.attribute("colspan", "2");
            writer.begin("span");
            writer.attribute(cls, "settingDescription");
            writer.print(better.getDescription(rawLabel));
            writer.end(); //span
            writer.end(); //td
            writer.end(); //tr
        }
    }
}
