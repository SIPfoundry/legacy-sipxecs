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
package org.sipfoundry.sipxconfig.cfgmgt;

import static java.lang.String.format;

import java.io.IOException;
import java.io.Writer;

/**
 * Only useful when utilities like Jackson JSON writer is an overkill and you want
 * something very lightweight. In fact useful when used in combination with jackson
 * Example:
 *
 *  JsonConfigurationFile conf = new JsonConfigurationFile(out);
 *  ObjectMapper jackson = new ObjectMapper();
 *  conf.open("planCandidates");
 *  jackson.writeValue(out, plans);
 *  conf.close();
 *  conf.open("regionCandidates");
 *  jackson.writeValue(out, regions);
 *  conf.close();
 */
public class JsonConfigurationFile {
    private Writer m_out;
    private boolean m_needComma;

    public JsonConfigurationFile(Writer w) {
        m_out = w;
    }

    public void open(String name) throws IOException {
        if (m_needComma) {
            m_out.write(",\n");
        } else {
            m_out.write("{\n");
        }
        m_out.write(format("\"%s\" : ", name));
        m_needComma = false;
    }

    public void close() throws IOException {
        if (m_needComma) {
            m_out.write("\n}");
        }
        m_needComma = true;
    }
}
