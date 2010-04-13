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
import java.util.Locale;
import java.util.MissingResourceException;
import java.util.ResourceBundle;

import org.apache.commons.io.FilenameUtils;
import org.springframework.context.support.ResourceBundleMessageSource;

/**
 * Special type of message source that finds it's resource bundle in the same plae where model
 * files are stored.
 */
public class ModelMessageSource extends ResourceBundleMessageSource {
    public ModelMessageSource(File modelFile) {
        setBasename(getBundleBasename(modelFile));
        setBundleClassLoader(getClass().getClassLoader());
    }

    /**
     * Find name for message bundle based on the name of the model file.
     *
     * The assumption here is that the parent directory is in the classpath. If the name of the
     * file is '/etc/sipxpbx/polycom/phone.xml' we conver it to polycom.phone because we assume
     * that /etc/sipxpbx is in the classpath.
     *
     * This will only work of course if model files are kept in subdirectories. It would be better
     * if both model class and resource bundles were loaded as resources using the same mechanism.
     *
     * @param modelFile name of the .xml file containing the phone model, this is usually a name
     *        somewhere in /etc/sipxpbx directory
     *
     * @return name of the bundle which has to look like a class name hence '.' as an separator
     */
    private String getBundleBasename(File modelFile) {
        String parentName = modelFile.getParentFile().getName();
        String baseName = FilenameUtils.getBaseName(modelFile.getName());
        return parentName + "." + baseName;
    }

    /**
     * Handle MissignResourceException - base class logs scary looking error on the WARN level. We
     * actually OK with having this on INFO level, since not having a bundle for model is OK in
     * most cases.
     */
    protected ResourceBundle doGetBundle(String basename, Locale locale) {
        try {
            return super.doGetBundle(basename, locale);
        } catch (MissingResourceException e) {
            if (logger.isDebugEnabled()) {
                logger.debug("ResourceBundle [" + basename + "] not found for MessageSource: " + e.getMessage());
            }
            return null;
        }

    }

}
