package org.sipfoundry.commons.timeout;

public class Result {
    private boolean m_succesfull;
    private Object m_result;

    public Result (boolean succesfull, Object result) {
        m_succesfull = succesfull;
        m_result = result;
    }

    public boolean isSuccesfull() {
        return m_succesfull;
    }
    public void setSuccesfull(boolean succesfull) {
        m_succesfull = succesfull;
    }
    public Object getResult() {
        return m_result;
    }
    public void setResult(Object result) {
        m_result = result;
    }
}
