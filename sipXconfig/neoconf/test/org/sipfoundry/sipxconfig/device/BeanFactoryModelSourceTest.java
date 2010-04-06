/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.device;

import java.util.Iterator;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.springframework.beans.factory.ListableBeanFactory;

public class BeanFactoryModelSourceTest extends TestCase {

    public void testGetSortModels() {
        String[] birdNames = {
                "bufflehead", "towhee", "bobolink", "grasshopper sparrow"
        };
        IMocksControl beanFactoryControl = EasyMock.createControl();
        ListableBeanFactory beanFactory = beanFactoryControl.createMock(ListableBeanFactory.class);
        beanFactory.getBeanNamesForType(BirdType.class);
        beanFactoryControl.andReturn(birdNames);
        for (String name : birdNames) {
            beanFactory.getBean(name);
            beanFactoryControl.andReturn(new BirdType(name));
        }
        beanFactoryControl.replay();
        BeanFactoryModelSource<BirdType> modelSource = new BeanFactoryModelSource(BirdType.class.getName());
        modelSource.setBeanFactory(beanFactory);
        Iterator<BirdType> birds = modelSource.getModels().iterator();
        assertEquals("bobolink", birds.next().getLabel());
        assertEquals("bufflehead", birds.next().getLabel());
        assertEquals("grasshopper sparrow", birds.next().getLabel());
        assertEquals("towhee", birds.next().getLabel());

        beanFactoryControl.verify();
    }

    public static class BirdType extends DeviceDescriptor {
        BirdType(String name) {
            setModelId(name);
            setLabel(name);
        }
    }

}
