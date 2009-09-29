/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.api;

import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

import junit.framework.TestCase;

public class ApiBeanUtilTest extends TestCase {
    private Set frackProps;

    protected void setUp() {
        frackProps = new HashSet();
    }

    public void testCopyProperties() {
        Frick frick = new Frick("yaya", "123 hello st.");
        Frack frack = new Frack();
        frackProps.add("name");
        ApiBeanUtil.copyProperties(frack, frick, frackProps, null);
        assertEquals("yaya", frack.getName());
    }

    public void testReadableProperties() {
        Object[] props = ApiBeanUtil.getProperties(new Frick()).toArray();
        Arrays.sort(props);
        assertEquals(2, props.length);
        assertEquals("address", props[0]);
        assertEquals("name", props[1]);
    }

    public void testToApiArray() {
        Frick[] fricks = new Frick[] {
                new Frick("a", "b"),
                new Frick("c", "d")
        };
        SimpleBeanBuilder builder = new SimpleBeanBuilder();
        builder.getCustomFields().add("abode");
        Frack[] fracks = (Frack[]) ApiBeanUtil.toApiArray(builder, fricks, Frack.class);
        checkFracks(fracks);
        fracks = (Frack[]) ApiBeanUtil.toApiArray(builder, Arrays.asList(fricks), Frack.class);
        checkFracks(fracks);
    }

    private void checkFracks(Frack[] fracks) {
        assertEquals("a", fracks[0].getName());
        assertEquals(null, fracks[0].getAbode());
        assertEquals("c", fracks[1].getName());
        assertEquals(null, fracks[1].getAbode());
    }

    public void testToMyArray() {
        Frack[] fracks = new Frack[] {
                new Frack("a", "b"),
                new Frack("c", "d")
        };
        SimpleBeanBuilder builder = new SimpleBeanBuilder();
        builder.getCustomFields().add("abode");
        Frick[] fricks = (Frick[]) ApiBeanUtil.toMyArray(builder, fracks, Frick.class);
        checkFricks(fricks);
        fricks = (Frick[]) ApiBeanUtil.toMyArray(builder, Arrays.asList(fracks), Frick.class);
        checkFricks(fricks);
    }

    public void checkFricks(Frick[] fricks) {
        assertEquals("a", fricks[0].getName());
        assertEquals(null, fricks[0].getAddress());
        assertEquals("c", fricks[1].getName());
        assertEquals(null, fricks[1].getAddress());
    }

    public void testFindProperty() {
        Property[] props = new Property[] {
                new Property("feathers", "purple"),
                new Property("beak", "2 inches")
        };
        assertSame(props[1], ApiBeanUtil.findProperty(props, "beak"));
        assertNull(ApiBeanUtil.findProperty(props, "tail"));
    }
}