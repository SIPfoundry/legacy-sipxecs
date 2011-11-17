package org.sipfoundry.sipxconfig.openacd;

import java.util.Collections;
import java.util.regex.Pattern;

import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchCondition;

import junit.framework.TestCase;

public class OpenAcdLineTest extends TestCase{

    public void testRegularExpression() {
        OpenAcdLine line = new OpenAcdLine();
        
        assertNull(line.getCapturedExtension());
        
        FreeswitchCondition condition = new FreeswitchCondition();
        condition.setField(OpenAcdExtension.DESTINATION_NUMBER);
        condition.setExpression("501");
        
        line.setConditions(Collections.singleton(condition));
        
        assertEquals("501", line.getExtension());
        assertEquals("501", line.getCapturedExtension());
        
        condition.setRegex(true);
        
        line.setConditions(Collections.singleton(condition));
        
        assertEquals("501", line.getExtension());
        
        try {
            line.getCapturedExtension();
        } catch (UserException e) {
            assertEquals("&error.regex.no.valid.group", e.getMessage());
        }
        
        condition.setExpression("(501)");
        line.setConditions(Collections.singleton(condition));
        assertEquals("501", line.getCapturedExtension());
        condition.setExpression("(501);cic=(\\d{1,8})");
        line.setConditions(Collections.singleton(condition));
        assertEquals("501", line.getCapturedExtension());
        
        condition.setExpression("(501)\\a");
        line.setConditions(Collections.singleton(condition));

        try {
            line.getCapturedExtension();
        } catch (UserException e) {
            assertEquals("&error.regex.invalid", e.getMessage());
        }
        
        condition.setExpression("(501)(;cic=(\\d{1,8}))?");
        line.setConditions(Collections.singleton(condition));
        assertTrue(Pattern.matches(condition.getExtension(), "501")); 
        assertTrue(Pattern.matches(condition.getExtension(), "501;cic=123"));
        assertTrue(Pattern.matches(condition.getExtension(), "501;cic=12345678"));
        assertFalse(Pattern.matches(condition.getExtension(), "501;cic=123456789"));
    }
}
