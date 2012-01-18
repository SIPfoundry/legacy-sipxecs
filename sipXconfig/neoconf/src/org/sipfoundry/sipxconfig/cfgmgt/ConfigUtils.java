/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
    public static final void enableCfengineClass(File dir, String file, String property, boolean enabled)
        throws IOException {
        Writer w = new FileWriter(new File(dir, file));
        try {
            CfengineModuleConfiguration config = new CfengineModuleConfiguration(w);
            config.writeClass(property, enabled);
        } finally {
            IOUtils.closeQuietly(w);
        }
    }
}
