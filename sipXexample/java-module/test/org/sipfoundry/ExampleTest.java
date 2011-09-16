package org.sipfoundry;

import static org.junit.Assert.assertTrue;

import org.junit.Test;
import org.sipfoundry.Example;

public class ExampleTest {

    @Test
    public void hello() {
        assertTrue(new Example().hello());
    }
}
