/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;

import org.apache.commons.io.IOUtils;

/**
 * Helpful utilities generating configuration files
 */
public final class ConfigUtils {

    private ConfigUtils() {
    }

    /**
     * Writes a file with a single line that translated to on/off in cf scripts
     * Example:
     *
     * In Java
     *   ConfigUtils.enableCfengineClass(dir, "foo.cfdat", "goose", true);
     *
     * In CFEngine script
     *
     *   bundle agent Bird {
     *      reports:
     *        goose::
     *           "Goose is enabled!";
     *        !goose::
     *           "Goose is not enabled!";
     *   }
     */
    public static final void enableCfengineClass(File dir, String file, boolean enabled, String... properties)
        throws IOException {
        Writer w = new FileWriter(new File(dir, file));
        try {
            CfengineModuleConfiguration config = new CfengineModuleConfiguration(w);
            for (String property : properties) {
                config.writeClass(property, enabled);
            }
        } finally {
            IOUtils.closeQuietly(w);
        }
    }
}
