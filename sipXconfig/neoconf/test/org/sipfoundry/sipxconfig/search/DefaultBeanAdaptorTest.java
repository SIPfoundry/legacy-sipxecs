/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.search;

import junit.framework.TestCase;

import org.apache.lucene.document.Document;
import org.apache.lucene.document.Field;
import org.apache.lucene.index.Term;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.acme.AcmePhone;
import org.sipfoundry.sipxconfig.search.BeanAdaptor.Identity;

public class DefaultBeanAdaptorTest extends TestCase {
    private DefaultBeanAdaptor m_adaptor;

    @Override
    protected void setUp() throws Exception {
        m_adaptor = new DefaultBeanAdaptor();
    }

    public void testGetBeanIndentity() throws Exception {
        // cannot mock documents
        Document doc = new Document();
        doc.add(new Field("id", "org.sipfoundry.sipxconfig.common.User:36", Field.Store.YES,
                Field.Index.UN_TOKENIZED));

        Identity beanIdentity = m_adaptor.getBeanIdentity(doc);
        assertSame(User.class, beanIdentity.getBeanClass());
        assertEquals(new Integer(36), beanIdentity.getBeanId());
    }

    public void testGetBeanIndentityWrongClass() throws Exception {
        // cannot mock documents
        Document doc = new Document();
        doc.add(new Field("id", "org.sipfoundry.sipxconfig.common.Xyz:36", Field.Store.YES,
                Field.Index.UN_TOKENIZED));

        Identity beanIdentity = m_adaptor.getBeanIdentity(doc);
        assertNull(beanIdentity);
    }

    public void testGetBeanIndentityWrongId() throws Exception {
        // cannot mock documents
        Document doc = new Document();
        doc.add(new Field("id", "org.sipfoundry.sipxconfig.common.User:aaa", Field.Store.YES,
                Field.Index.UN_TOKENIZED));

        Identity beanIdentity = m_adaptor.getBeanIdentity(doc);
        assertNull(beanIdentity);
    }

    public void testGetIndentityTerm() throws Exception {
        User user = new User();
        user.setUniqueId();

        Term identityTerm = m_adaptor.getIdentityTerm(user, user.getId());

        Term expectedTerm = new Term("id", "org.sipfoundry.sipxconfig.common.User:"
                + user.getId());
        assertEquals(expectedTerm, identityTerm);
    }

    public void testIndexClass() {
        Document doc = new Document();
        m_adaptor.indexClass(doc, User.class);

        assertEquals(User.class.getName(), doc.get(Indexer.CLASS_FIELD));

        doc = new Document();
        m_adaptor.indexClass(doc, AcmePhone.class);
        // snom phone should be indexed as phone
        assertEquals(Phone.class.getName(), doc.get(Indexer.CLASS_FIELD));

        doc = new Document();
        m_adaptor.indexClass(doc, String.class);

        assertNull(doc.get(Indexer.CLASS_FIELD));
    }

    public void test() {
        Document doc = new Document();
        doc.add(new Field("name", "abc", Field.Store.YES, Field.Index.TOKENIZED));
        doc.add(new Field("extension", "1234", Field.Store.YES, Field.Index.TOKENIZED));
        doc.add(new Field("description", "bongo", Field.Store.YES, Field.Index.TOKENIZED));

        doc.add(new Field("id", "org.sipfoundry.sipxconfig.common.User:36", Field.Store.YES,
                Field.Index.UN_TOKENIZED));

        Identity beanIdentity = m_adaptor.getBeanIdentity(doc);
        assertEquals("bongo", beanIdentity.getDescription());
        assertEquals("abc, 1234", beanIdentity.getName());
    }
}
