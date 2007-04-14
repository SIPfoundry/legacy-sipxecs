/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.bulk.ldap;

import java.util.Arrays;
import java.util.Collections;

import junit.framework.TestCase;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.bulk.ldap.Schema.ClassDefinition;

public class SchemaTest extends TestCase {

    public void testAddClassDefinitionFull() {
        String definition = "( 2.5.6.6 NAME 'person' DESC 'RFC2256: a person' SUP top "
                + "STRUCTURAL MUST ( sn $ cn ) MAY ( userPassword $ telephoneNumber $ seeAlso $ description ) )";

        Schema schema = new Schema();
        assertNull(schema.getAttributes("person"));
        schema.addClassDefinition(definition);

        String expectedAttributesString = "sn cn userPassword telephoneNumber seeAlso description";

        String[] attributes = schema.getAttributes("person");

        String[] expected = StringUtils.split(expectedAttributesString);
        assertTrue(Arrays.equals(attributes, expected));
    }

    public void testGetObjectClassesNames() {
        Schema schema = new Schema();
        assertEquals(0, schema.getObjectClassesNames().length);
        schema.addClassDefinition("( 2.5.6.6 NAME 'bongo' DESC ''");
        schema.addClassDefinition("( 2.5.6.6 NAME 'kuku' DESC ''");

        String[] classesNames = schema.getObjectClassesNames();

        assertEquals(2, classesNames.length);
        assertTrue(ArrayUtils.indexOf(classesNames, "bongo") >= 0);
        assertTrue(ArrayUtils.indexOf(classesNames, "kuku") >= 0);
    }

    public void testFromClassDefinitionString() {
        String definition = "( 1.3.6.1.1.1.2.8 NAME 'nisNetgroup' DESC 'Abstraction of a netgroup' SUP top "
                + "STRUCTURAL MUST cn MAY ( isNetgroupTriple $ memberNisNetgroup $ description ) )";

        ClassDefinition cd = ClassDefinition.fromSchemaString(definition);
        assertEquals("Abstraction of a netgroup", cd.getDescription());
        assertEquals("nisNetgroup", cd.getName());
        assertEquals("top", cd.getSup());

        String expectedMustString = "cn";
        assertEquals(1, cd.getMust().length);
        assertEquals(expectedMustString, StringUtils.join(cd.getMust(), " "));

        String expectedMayString = "isNetgroupTriple memberNisNetgroup description";
        assertEquals(3, cd.getMay().length);
        assertEquals(expectedMayString, StringUtils.join(cd.getMay(), " "));
    }

    public void testFromClassDefinitionStringNoMust() {
        String definition = "( 1.3.6.1.1.1.2.11 NAME 'ieee802Device' DESC 'A device with a MAC address' SUP top "
                + "AUXILIARY MAY macAddress )";
        ClassDefinition cd = ClassDefinition.fromSchemaString(definition);
        assertEquals("A device with a MAC address", cd.getDescription());
        assertEquals("ieee802Device", cd.getName());
        assertEquals("top", cd.getSup());
        assertEquals(0, cd.getMust().length);
        assertEquals(1, cd.getMay().length);
        assertEquals("macAddress", cd.getMay()[0]);
    }
    
    
    public void testGetAttributesPool() throws Exception {
        Schema schema = new Schema();
        assertEquals(0, schema.getAttributesPool(Collections.singleton("person")).length);

        schema.addClassDefinition("( 1.3 NAME 'x' DESC '' SUP top "
                + "AUXILIARY MUST a1 MAY a2 )");
        schema.addClassDefinition("( 1.3 NAME 'y' DESC '' SUP top "
                + "AUXILIARY MUST a1 MAY a3 )");
        schema.addClassDefinition("( 1.3 NAME 'z' DESC '' SUP top "
                + "AUXILIARY MUST a1 MAY a4 )");
        
        
        String[] attributesPool = schema.getAttributesPool(Arrays.asList(new String[] { "x", "z" }));
        
        assertEquals(3, attributesPool.length);
        assertEquals("a1a2a4", StringUtils.join(attributesPool));        
    }

}
