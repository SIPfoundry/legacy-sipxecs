/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.rest;

import static junit.framework.TestCase.assertEquals;
import static junit.framework.TestCase.assertFalse;

import java.io.IOException;
import java.io.InputStream;
import java.io.Serializable;
import java.io.StringWriter;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.junit.Test;
import org.restlet.data.MediaType;
import org.restlet.resource.InputRepresentation;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.StringRepresentation;

public class JacksonConvertTest {

    @Test
    public void testFromSimpleObject() {
        SimpleObject expected = new SimpleObject();
        expected.setName("simple object");
        boolean exceptionThrown = false;
        try {
            InputStream is = getClass().getResourceAsStream("jackson-convert-simple.test.json");
            Representation entity = new InputRepresentation(is, MediaType.APPLICATION_JSON);
            SimpleObject actual = JacksonConvert.fromRepresentation(entity, SimpleObject.class);
            assertEquals(expected, actual);
            IOUtils.closeQuietly(is);
        } catch (ResourceException e) {
            exceptionThrown = true;
        }

        assertFalse(exceptionThrown);
    }

    @Test
    public void testToSimpleObject() throws IOException {
        SimpleObject sObj = new SimpleObject();
        sObj.setName("simple object");
        boolean exceptionThrown = false;
        try {
            InputStream is = getClass().getResourceAsStream("jackson-convert-simple.test.json");
            StringWriter writer = new StringWriter();
            IOUtils.copy(is, writer, "UTF-8");
            String jsonStr = writer.toString();
            Representation expected = new StringRepresentation(jsonStr, MediaType.APPLICATION_JSON);
            Representation actual = JacksonConvert.toRepresentation(sObj);
            assertEquals(expected.getText(), actual.getText());
            IOUtils.closeQuietly(is);
        } catch (ResourceException e) {
            exceptionThrown = true;
        }

        assertFalse(exceptionThrown);
    }

    @Test
    public void testFromObjectWList() {
        ObjectWList expected = new ObjectWList();
        expected.setName("object w/ list");
        expected.setList(Arrays.asList(new String[] {
            "str1", "str2"
        }));
        boolean exceptionThrown = false;
        try {
            InputStream is = getClass().getResourceAsStream("jackson-convert-w-list.test.json");
            Representation entity = new InputRepresentation(is, MediaType.APPLICATION_JSON);
            ObjectWList actual = JacksonConvert.fromRepresentation(entity, ObjectWList.class);
            assertEquals(expected, actual);
            IOUtils.closeQuietly(is);
        } catch (ResourceException e) {
            exceptionThrown = true;
        }

        assertFalse(exceptionThrown);
    }

    @Test
    public void testToObjectWList() throws IOException {
        ObjectWList obj = new ObjectWList();
        obj.setName("object w/ list");
        obj.setList(Arrays.asList(new String[] {
            "str1", "str2"
        }));
        boolean exceptionThrown = false;
        try {
            InputStream is = getClass().getResourceAsStream("jackson-convert-w-list.test.json");
            StringWriter writer = new StringWriter();
            IOUtils.copy(is, writer, "UTF-8");
            String jsonStr = writer.toString();
            Representation expected = new StringRepresentation(jsonStr, MediaType.APPLICATION_JSON);
            Representation actual = JacksonConvert.toRepresentation(obj);
            assertEquals(expected.getText(), actual.getText());
            IOUtils.closeQuietly(is);
        } catch (ResourceException e) {
            exceptionThrown = true;
        }

        assertFalse(exceptionThrown);
    }

    private static class SimpleObject implements Serializable {
        /**
         * We just need this to create an ObjectRepresentation<SimpleObject>
         */
        private static final long serialVersionUID = 1L;
        private String name;

        @SuppressWarnings("unused")
        public String getName() {
            return name;
        }

        public void setName(String name) {
            this.name = name;
        }

        @Override
        public int hashCode() {
            final int prime = 31;
            int result = 1;
            result = prime * result + ((name == null) ? 0 : name.hashCode());
            return result;
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj) {
                return true;
            }
            if (obj == null) {
                return false;
            }
            if (getClass() != obj.getClass()) {
                return false;
            }
            SimpleObject other = (SimpleObject) obj;
            if (name == null) {
                if (other.name != null) {
                    return false;
                }
            } else if (!name.equals(other.name)) {
                return false;
            }
            return true;
        }
    }

    private static class ObjectWList implements Serializable {
        /**
         * We just need this to create an ObjectRepresentation<SimpleObject>
         */
        private static final long serialVersionUID = 1L;
        private String name;
        private List<String> list;

        @SuppressWarnings("unused")
        public String getName() {
            return name;
        }

        public void setName(String name) {
            this.name = name;
        }

        @SuppressWarnings("unused")
        public List<String> getList() {
            return list;
        }

        public void setList(List<String> list) {
            this.list = list;
        }

        @Override
        public int hashCode() {
            final int prime = 31;
            int result = 1;
            result = prime * result + ((list == null) ? 0 : list.hashCode());
            result = prime * result + ((name == null) ? 0 : name.hashCode());
            return result;
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj) {
                return true;
            }
            if (obj == null) {
                return false;
            }
            if (getClass() != obj.getClass()) {
                return false;
            }
            ObjectWList other = (ObjectWList) obj;
            if (list == null) {
                if (other.list != null) {
                    return false;
                }
            } else if (!list.equals(other.list)) {
                return false;
            }
            if (name == null) {
                if (other.name != null) {
                    return false;
                }
            } else if (!name.equals(other.name)) {
                return false;
            }
            return true;
        }

    }
}
