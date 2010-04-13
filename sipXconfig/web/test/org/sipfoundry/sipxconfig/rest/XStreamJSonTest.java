/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.rest;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.io.IOUtils;

import com.thoughtworks.xstream.XStream;
import com.thoughtworks.xstream.converters.UnmarshallingContext;
import com.thoughtworks.xstream.converters.collections.CollectionConverter;
import com.thoughtworks.xstream.io.HierarchicalStreamReader;
import com.thoughtworks.xstream.io.json.JettisonMappedXmlDriver;
import com.thoughtworks.xstream.io.json.JsonHierarchicalStreamDriver;
import com.thoughtworks.xstream.mapper.Mapper;

import junit.framework.TestCase;

/**
 * Does not really test sipXconfig: test xstream and json libraries that we are trying to use to
 * implement tests
 */
public class XStreamJSonTest extends TestCase {

    /**
     * Writes JSON using hierarchical driver, reads it using jettison
     */
    public void testJsonStoreAndParseMixed() throws Exception {
        XStream xs = new XStream(new JsonHierarchicalStreamDriver());
        xs.alias("car", Car.class);
        xs.alias("wheel", Wheel.class);

        Car car = new Car("Opel");
        String json = xs.toXML(car);

        // System.err.println(json);
        final InputStream jsonStream = getClass().getResourceAsStream("car.xstream.test.json");
        assertEquals(IOUtils.toString(jsonStream), json);

        XStream xsout = new XStream(new JettisonMappedXmlDriver());
        xsout.alias("car", Car.class);
        xsout.alias("wheel", Wheel.class);
        // not needed: xsout.registerLocalConverter(Car.class, "wheels", new
        // WheelsConverter(xsout.getMapper()));
        xsout.addImplicitCollection(Car.class, "wheels", Wheel.class);

        Car carOut = (Car) xsout.fromXML(json);
        assertEquals(4, carOut.getWheelCount());
        assertEquals("Opel0123", carOut.dump());
    }

    /**
     * Writes and reads JSON using jettison
     */
    public void testJsonStoreAndParseJettison() throws Exception {
        Car car = new Car("Opel");

        XStream xsout = new XStream(new JettisonMappedXmlDriver());
        xsout.alias("car", Car.class);
        xsout.alias("wheel", Wheel.class);
        // please not how we are forcing JSON writer to emit "wheels" as an item tag
        // we want to be able to write car.wheels[3] in javascript...
        xsout.addImplicitCollection(Car.class, "wheels", "wheels", Wheel.class);
        String json = xsout.toXML(car);
        // System.err.println(json);

        final InputStream jsonStream = getClass().getResourceAsStream("car.jettison.test.json");
        assertEquals(IOUtils.toString(jsonStream), json);

        Car carOut = (Car) xsout.fromXML(json);
        assertEquals(4, carOut.getWheelCount());
        assertEquals("Opel0123", carOut.dump());
    }
}

class WheelsConverter extends CollectionConverter {
    public WheelsConverter(Mapper mapper) {
        super(mapper);
    }

    @Override
    protected Object readItem(HierarchicalStreamReader reader, UnmarshallingContext context, Object current) {
        return context.convertAnother(current, Wheel.class);
    }
}

class Car {
    private final String model;
    private final List<Wheel> wheels;

    public Car(String m) {
        model = m;
        wheels = new ArrayList<Wheel>();
        for (int i = 0; i < 4; i++) {
            wheels.add(new Wheel(i));
        }
    }

    public int getWheelCount() {
        return wheels.size();
    }

    public String dump() {
        StringBuilder str = new StringBuilder(model);
        for (Wheel wheel : wheels) {
            str.append(wheel.getPosition());
        }
        return str.toString();
    }
}

class Wheel {
    private final int position;
    @SuppressWarnings("unused")
    private final String str;

    public Wheel(int p) {
        position = p;
        str = (p % 2 == 0) ? "even" : "odd";
    }

    public int getPosition() {
        return position;
    }
}
