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
package org.sipfoundry.sipxconfig.site.common;

import org.apache.commons.lang.StringUtils;
import org.apache.hivemind.Location;
import org.apache.hivemind.Resource;
import org.apache.hivemind.impl.LocationImpl;
import org.apache.tapestry.INamespace;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.resolver.ISpecificationResolverDelegate;
import org.apache.tapestry.services.ClassFinder;
import org.apache.tapestry.services.ClasspathResourceFactory;
import org.apache.tapestry.spec.ComponentSpecification;
import org.apache.tapestry.spec.IComponentSpecification;

public class PluginSpecificationResolver implements ISpecificationResolverDelegate {

    private ClassFinder m_clazzFinder;
    private ClasspathResourceFactory m_resourceFactory;

    public void setClazzFinder(ClassFinder clazzFinder) {
        this.m_clazzFinder = clazzFinder;
    }

    public void setClasspathResourceFactory(ClasspathResourceFactory resourceFactory) {
        m_resourceFactory = resourceFactory;
    }

    public IComponentSpecification findPageSpecification(IRequestCycle cycle, INamespace namespace,
            String simplePageName) {
        return findPluginSpecification(namespace, simplePageName, ".page");
    }

    public IComponentSpecification findComponentSpecification(IRequestCycle cycle, INamespace namespace, String type) {
        return findPluginSpecification(namespace, type, ".jwc");
    }

    private IComponentSpecification findPluginSpecification(INamespace namespace, String name, String type) {
        String packages = namespace.getPropertyValue("org.apache.tapestry.plugin-packages");
        String className = StringUtils.removeStart(name, "plugin/").replace('/', '.');
        Class pageClass = m_clazzFinder.findClass(packages, className);
        if (pageClass == null) {
            return null;
        }

        IComponentSpecification spec = new ComponentSpecification();
        Resource componentResource = m_resourceFactory.newResource(name + type);
        Location location = new LocationImpl(componentResource);
        spec.setLocation(location);
        spec.setSpecificationLocation(componentResource);
        spec.setComponentClassName(pageClass.getName());
        return spec;
    }
}
