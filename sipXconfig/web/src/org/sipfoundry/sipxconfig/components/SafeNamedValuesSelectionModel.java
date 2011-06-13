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
package org.sipfoundry.sipxconfig.components;

import java.util.Map;

/**
 * Uses pairs of option->label as a backend for Tapestry implementation
 */
public class SafeNamedValuesSelectionModel extends NamedValuesSelectionModel {

    /**
     * @map if instance of commons LinkedMap, use it directly, otherwise copy into a order-aware
     *      map
     */
    public SafeNamedValuesSelectionModel(Map< ? , String> map) {
        super(map);
    }

    public SafeNamedValuesSelectionModel(Object[] options, String[] labels) {
        super(options, labels);
    }

    @Override
    public Object getOption(int index) {
        if (index >= getOptions().length) {
            return getOptions()[0];
        }
        return getOptions()[index];
    }

    @Override
    public String getLabel(int index) {
        if (index >= getLabels().length) {
            return getLabels()[0];
        }
        return getLabels()[index];
    }
}
