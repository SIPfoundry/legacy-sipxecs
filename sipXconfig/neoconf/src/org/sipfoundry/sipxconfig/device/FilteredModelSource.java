/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.device;

import java.util.Collection;
import java.util.regex.Pattern;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Predicate;

public class FilteredModelSource<T extends Model> implements ModelSource {

    private Predicate m_filter;

    private ModelSource<T> m_modelSource;

    public void setModelSource(ModelSource modelSource) {
        m_modelSource = modelSource;
    }

    /**
     * Sets regular expression that is used to construct the filter limiting which models are
     * available.
     */
    public void setCertified(String certifiedRegex) {
        m_filter = new CertifiedPhones(certifiedRegex);
    }

    public void setFilter(Predicate filter) {
        m_filter = filter;
    }

    public Collection<T> getModels() {
        Collection<T> models = m_modelSource.getModels();
        if (m_filter != null) {
            models = CollectionUtils.select(models, m_filter);
        }
        return models;
    }

    private static class CertifiedPhones implements Predicate {
        private Pattern m_pattern;

        public CertifiedPhones(String pattern) {
            m_pattern = Pattern.compile(pattern);
        }

        public boolean evaluate(Object object) {
            DeviceDescriptor model = (DeviceDescriptor) object;
            return m_pattern.matcher(model.getModelId()).matches();
        }
    }

    public T getModel(String modelId) {
        return m_modelSource.getModel(modelId);
    }
}
