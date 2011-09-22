package org.sipfoundry.commons.util;

import java.io.PrintStream;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.junit.internal.JUnitSystem;
import org.junit.internal.RealSystem;
import org.junit.internal.TextListener;
import org.junit.runner.Description;
import org.junit.runner.JUnitCore;
import org.junit.runner.Result;
import org.junit.runner.notification.Failure;
import org.junit.runner.notification.RunListener;

public class VerboseJUnitRunner {

    public static void main(String... args) {
        JUnitCore junit = new JUnitCore();
        JUnitSystem system = new RealSystem();
        String brokenTest = System.getProperty("brokentest");
        if (StringUtils.isNotBlank(brokenTest)) {
            junit.addListener(new BrokenTestListener(system, brokenTest));
            junit.addListener(new VerboseListener(system));
        } else {
            if (Boolean.getBoolean(System.getProperty("verbose", "false"))) {
                junit.addListener(new VerboseListener(system));
            } else {
                junit.addListener(new TextListener(system));
            }
        }
        List<Class< ? >> classes = new ArrayList<Class< ? >>();
        for (String each : args) {
            try {
                if (!each.equals(brokenTest)) {
                    classes.add(Class.forName(each));
                    if (StringUtils.isNotBlank(brokenTest)) {
                        classes.add(Class.forName(brokenTest));                
                    }
                }
            } catch (ClassNotFoundException e) {
                throw new RuntimeException(e);
            }
        }
        Result result = junit.run(classes.toArray(new Class[0]));
        system.exit(result.wasSuccessful() ? 0 : 1);
    }
    
    static final class BrokenTestListener extends RunListener {
        private PrintStream m_out;
        private String m_lastTest;
        private String m_brokenTest;
        private boolean m_foundBreakage;

        public BrokenTestListener(JUnitSystem system, String brokenTest) {
            m_out = system.out();
            m_brokenTest = brokenTest;
        }

        public void testFinished(Description description) {
            if (!m_foundBreakage) {
                if (!description.getClassName().equals(m_brokenTest)) {
                    m_lastTest = description.getClassName();
                }
            }
        }
        
        public void testFailure(Failure failure) {
            if (failure.getDescription().getClassName().equals(m_brokenTest)) {
                if (!m_foundBreakage) {
                    if (m_lastTest == null) {
                        throw new IllegalStateException("No previous tests run");
                    }
                    m_out.println(m_lastTest + " broke test " + m_brokenTest);                    
                }
                m_foundBreakage = true;
            }
        }
        
        public void testRunFinished(Result result) {
            if (m_foundBreakage) {
                m_out.println(m_lastTest + " broke test " + m_brokenTest);
            } else {
                m_out.println("No failed tests");
            }
        }
    }

    static final class VerboseListener extends TextListener {
        private PrintStream m_out;
        List<String> m_failed = new ArrayList<String>();

        public VerboseListener(JUnitSystem system) {
            super(system);
            m_out = system.out();
        }

        public void testStarted(Description description) {
            m_out.println('[' + description.getClassName() + ':' + description.getMethodName() + ']');
        }

        public void testFailure(Failure failure) {
            m_failed.add(failure.getDescription().getClassName());
            m_out.println("[FAILED]");
        }
        
        public void printSummmary() {
            if (!m_failed.isEmpty()) {
                m_out.println("Failed Tests");
                m_out.println(StringUtils.join(m_failed, ' '));
            }
        }
    }
}
