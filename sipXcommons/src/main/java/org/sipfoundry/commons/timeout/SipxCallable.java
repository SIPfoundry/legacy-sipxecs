package org.sipfoundry.commons.timeout;

import java.util.concurrent.Callable;

public class SipxCallable implements Callable<Object>{
    Timeout m_timeout;

    public SipxCallable(Timeout timeout) {
        m_timeout = timeout;
    }
    @Override
    public Result call() throws Exception {
        // TODO Auto-generated method stub
        return m_timeout.timeoutMethod();
    }

}
