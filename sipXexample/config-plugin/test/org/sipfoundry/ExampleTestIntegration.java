package org.sipfoundry;

import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.springframework.context.ConfigurableApplicationContext;
import org.springframework.context.support.ClassPathXmlApplicationContext;

public class ExampleTestIntegration extends IntegrationTestCase {    
    private Example m_example;
    
    @Override
    protected ConfigurableApplicationContext createApplicationContext(String[] locations) {
        List<String> jars = new ArrayList<String>();
        jars.add("classpath:/org/sipfoundry/sipxconfig/system.beans.xml");
        jars.add("classpath:/sipxplugin.beans.xml");
        jars.add("classpath*:/org/sipfoundry/sipxconfig/*/**/*.beans.xml");        
        return new ClassPathXmlApplicationContext(jars.toArray(new String[0]));
    }
    
    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }
    
    public void testHello() {
        // validates this test case has injected properly 
        assertEquals("hello world", m_example.hello());
    }

    public void setExample(Example example) {
        m_example = example;
    }
}
