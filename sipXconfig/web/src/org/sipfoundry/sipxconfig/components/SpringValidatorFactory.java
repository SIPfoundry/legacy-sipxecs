/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import java.util.ArrayList;
import java.util.List;

import org.apache.hivemind.lib.SpringBeanFactorySource;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.form.validator.Validator;
import org.apache.tapestry.form.validator.ValidatorFactory;
import org.springframework.beans.factory.BeanFactory;

/**
 * Use validators defined in spring, then consult hivemind
 */
public class SpringValidatorFactory implements ValidatorFactory {

    private SpringBeanFactorySource m_beanFactorySource;

    private ValidatorFactory m_hivemindValidatorFactory;

    public void setBeanFactorySource(SpringBeanFactorySource beanFactorySource) {
        m_beanFactorySource = beanFactorySource;
    }

    public List constructValidatorList(IComponent component, String specification) {
        String[] tokens = splitSpecification(specification);
        ArrayList validators = new ArrayList();
        BeanFactory factory = m_beanFactorySource.getBeanFactory();
        for (int i = 0; i < tokens.length; i++) {
            // FIXME: switch to look in hivemind first, then spring to ensure
            // beans such as "required" don't accidently get picked up as validators
            if (factory.containsBean(tokens[i])) {
                Validator v = (Validator) factory.getBean(tokens[i]);
                validators.add(v);
            } else {
                // delegate the "magic" to the hivemind
                List hivemindValidators = m_hivemindValidatorFactory.constructValidatorList(component, tokens[i]);
                validators.addAll(hivemindValidators);
            }
        }

        return validators;
    }

    String[] splitSpecification(String specification) {
        return specification.split("\\s*,\\s*");
    }

    public void setHivemindValidatorFactory(ValidatorFactory hivemindValidators) {
        m_hivemindValidatorFactory = hivemindValidators;
    }
}
