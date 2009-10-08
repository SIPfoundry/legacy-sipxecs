/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.rest;

import java.io.InputStream;

import junit.framework.TestCase;
import org.restlet.data.MediaType;
import org.restlet.resource.InputRepresentation;
import org.restlet.resource.Representation;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class PhonesResourceTest extends TestCase {

    private PhoneContext m_phoneContext;

    private ModelSource<PhoneModel> m_phoneModelSource;

    @Override
    protected void setUp() throws Exception {
        m_phoneModelSource = createMock(ModelSource.class);

        m_phoneContext = createMock(PhoneContext.class);

        Phone phone = new TestPhone();
        PhoneModel model = new TestPhoneModel();

        m_phoneModelSource.getModel(TestPhone.MODEL_ID);
        expectLastCall().andReturn(model);
        m_phoneContext.newPhone(model);
        expectLastCall().andReturn(phone);
        m_phoneContext.storePhone(phone);
        expectLastCall();

        m_phoneModelSource.getModel(TestPhone.MODEL_ID);
        expectLastCall().andReturn(model);
        m_phoneContext.newPhone(model);
        expectLastCall().andReturn(phone);
        m_phoneContext.storePhone(phone);
        expectLastCall().andThrow(new UserException("Duplicate serial number."));

        m_phoneModelSource.getModel("noSuchPhoneModel");
        expectLastCall().andThrow(new IllegalArgumentException("Invalid model."));

        m_phoneModelSource.getModel(TestPhone.MODEL_ID);
        expectLastCall().andReturn(model);
        m_phoneContext.newPhone(model);
        expectLastCall().andReturn(phone);
        m_phoneContext.storePhone(phone);
        expectLastCall();

        replay(m_phoneContext, m_phoneModelSource);
    }

    @Override
    protected void tearDown() throws Exception {
        verify(m_phoneContext, m_phoneModelSource);
    }

    public void testAcceptRepresentation() throws Exception {
        PhonesResource resource = new PhonesResource();
        resource.setPhoneModelSource(m_phoneModelSource);
        resource.setPhoneContext(m_phoneContext);

        final InputStream xmlStream = getClass().getResourceAsStream("phones.rest.test.xml");
        Representation entity = new InputRepresentation(xmlStream, MediaType.TEXT_XML);
        resource.acceptRepresentation(entity);
    }

    private class TestPhone extends Phone {
        public static final String BEAN_ID = "testPhone";
        public static final String MODEL_ID = "testPhoneModel";

        public TestPhone() {
            setBeanId(BEAN_ID);
            setModelId(MODEL_ID);
        }

        @Override
        protected LineInfo getLineInfo(Line line) {
            return null;
        }

        @Override
        protected void setLineInfo(Line line, LineInfo lineInfo) {
        }
    }

    public class TestPhoneModel extends PhoneModel {
        public TestPhoneModel() {
            setBeanId(TestPhone.BEAN_ID);
            setModelId(TestPhone.MODEL_ID);
        }
    }
}
