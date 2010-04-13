/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;


public interface ModelFilesContext {
    public Setting loadModelFile(String basename);

    public Setting loadModelFile(String basename, String manufacturer);

    public Setting loadDynamicModelFile(String basename, String manufacturer,
            SettingExpressionEvaluator evaluator);
}
