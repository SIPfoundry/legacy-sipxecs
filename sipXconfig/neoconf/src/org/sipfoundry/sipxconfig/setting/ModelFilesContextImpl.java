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

import java.io.File;
import java.util.Set;

public class ModelFilesContextImpl implements ModelFilesContext {
    private String m_configDirectory;

    private ModelBuilder m_modelBuilder;

    public Setting loadModelFile(String basename) {
        return loadModelFile(basename, null);
    }

    /**
     * loadModelFile("phone.xml", "cisco", new String[] { "7912" }) Loads settings model from XML
     * file
     *
     * The full path of the model file is: systemEtcDirectory/manufacturer/basename
     *
     * @return new copy of the settings model
     */
    public Setting loadModelFile(String basename, String manufacturer) {
        File modelFile = getModelFile(basename, manufacturer);
        SettingSet model = m_modelBuilder.buildModel(modelFile);
        return model.copy();
    }

    /**
     * return a deep copy of this setting set that evaluate if/unless expressions that are
     * contained in this set. Example: if Set = { "Hi" } then settings/groups with if="Hi" will be
     * included settings/groups with unless="Hi" will not be included
     */
    public Setting loadDynamicModelFile(String basename, String manufacturer, Set defines) {
        return loadDynamicModelFile(basename, manufacturer, new SimpleDefinitionsEvaluator(defines));
    }

    public Setting loadDynamicModelFile(String basename, String manufacturer,
            SettingExpressionEvaluator evalutor) {
        Setting master = loadModelFile(basename, manufacturer);
        if (master == null) {
            return null;
        }
        ConditionalSet conditional = (ConditionalSet) master;
        Setting model = conditional.evaluate(evalutor);
        // no need to create an extra copy of the model, evaluator is already doing it
        return model;
    }

    private File getModelDirectory(String manufacturer) {
        if (manufacturer == null) {
            return new File(m_configDirectory);
        }
        return new File(m_configDirectory, manufacturer);
    }

    /**
     * @param manufacturer null if model file is in root directory
     */
    public File getModelFile(String basename, String manufacturer) {
        File modelDir = getModelDirectory(manufacturer);
        return new File(modelDir, basename);
    }

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
    }

    public void setModelBuilder(ModelBuilder modelBuilder) {
        m_modelBuilder = modelBuilder;
    }
}
