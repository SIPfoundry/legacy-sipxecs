package org.sipfoundry.sipxconfig.validation;

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;
import static org.sipfoundry.sipxconfig.validation.Validate.ipv4AddrBlock;

import org.junit.Test;

public class ValidateTest {

    @Test
    public void maxLen() {
        Validate.maxLen("x", "1234567890", 10);
        try {
            Validate.maxLen("x", "1234567890", 9);
            fail("Should have failed as being too long");
        } catch (ValidateException expected) {
            assertTrue(true);
        }
    }

    @Test
    public void invalidIpv4AddrBlock() {
        String[] invalid = new String[] {
            "a", "1", "1.1.1", "1.1.1.1", "1.1.1.1/999", "1.1.1.1\\999"
        };
        for (String s : invalid) {
            try {
                ipv4AddrBlock("x", s);
                fail(s);
            } catch (ValidateException expected) {
                assertTrue(true);
            }
        }
    }

    @Test
    public void validIpv4AddrBlock() {
        ipv4AddrBlock("x", "1.1.1.1/32");
        ipv4AddrBlock("x", "1.1.1/32");
        ipv4AddrBlock("x", "1.1.1.1/8");
    }
}
