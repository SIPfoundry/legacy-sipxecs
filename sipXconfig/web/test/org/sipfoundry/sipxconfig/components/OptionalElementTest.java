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

import junit.framework.TestCase;

import org.apache.commons.beanutils.BeanUtils;
import org.apache.tapestry.IBinding;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.spec.IComponentSpecification;
import org.apache.tapestry.test.Creator;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;

public class OptionalElementTest extends TestCase {
    private Creator m_maker = new Creator();
    private OptionalElement m_oe;

    protected void setUp() throws Exception {
        m_oe = (OptionalElement) m_maker.newInstance(OptionalElement.class);
    }

    public void testRender() throws Exception {
        IMocksControl mcCycle = EasyMock.createControl();
        IRequestCycle cycle = mcCycle.createMock(IRequestCycle.class);
        cycle.isRewinding();
        mcCycle.andReturn(false).atLeastOnce();
        mcCycle.replay();

        IMocksControl mcWriter = EasyMock.createControl();
        IMarkupWriter writer = mcWriter.createMock(IMarkupWriter.class);

        mcWriter.replay();

        m_oe.setElement("");
        m_oe.renderComponent(writer, cycle);

        mcCycle.verify();
        mcWriter.verify();
    }

    public void testRenderWithElement() throws Exception {
        IMocksControl mcBinding = EasyMock.createControl();
        IBinding binding = mcBinding.createMock(IBinding.class);
        binding.getObject();
        mcBinding.andReturn("kuku").anyTimes();
        mcBinding.replay();

        IMocksControl mcCycle = EasyMock.createControl();
        IRequestCycle cycle = mcCycle.createMock(IRequestCycle.class);
        cycle.isRewinding();
        mcCycle.andReturn(false).atLeastOnce();
        mcCycle.replay();

        IMocksControl mcWriter = EasyMock.createStrictControl();
        IMarkupWriter writer = mcWriter.createMock(IMarkupWriter.class);
        writer.begin("bongo");
        writer.attribute("attr1", "kuku");
        writer.end("bongo");
        mcWriter.replay();

        IMocksControl mcComponentSpec = EasyMock.createNiceControl();
        IComponentSpecification componentSpec = mcComponentSpec.createMock(IComponentSpecification.class);
        mcComponentSpec.replay();
        // method available on proxy object, See Creator.java
        BeanUtils.setProperty(m_oe, "specification", componentSpec);

        m_oe.setBinding("attr1", binding);
        m_oe.setElement("bongo");
        m_oe.renderComponent(writer, cycle);

        mcBinding.verify();
        mcCycle.verify();
        mcWriter.verify();
        mcComponentSpec.verify();
    }
}
