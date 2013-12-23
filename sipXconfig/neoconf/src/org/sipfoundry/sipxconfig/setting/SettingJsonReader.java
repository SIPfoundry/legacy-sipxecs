/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.setting;

import java.util.Iterator;
import java.util.Map.Entry;

import org.codehaus.jackson.JsonNode;

public class SettingJsonReader {

    public void read(PersistableSettings settings, JsonNode node) {
        Iterator<Entry<String, JsonNode>> fields = node.getFields();
        while (fields.hasNext()) {
            Entry<String, JsonNode> setting = fields.next();
            settings.setSettingValue(setting.getKey(), setting.getValue().asText());
        }
    }
}
