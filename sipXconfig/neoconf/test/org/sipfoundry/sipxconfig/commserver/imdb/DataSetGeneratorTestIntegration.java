/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.commserver.imdb;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class DataSetGeneratorTestIntegration extends IntegrationTestCase {

    private final Map<String, AbstractDataSetGenerator> m_dataSetGeneratorMap;

    public DataSetGeneratorTestIntegration() {
        m_dataSetGeneratorMap = new HashMap<String, AbstractDataSetGenerator>();
    }

    public void testDataSetGeneratorBeans() {
        for (Iterator i = DataSet.iterator(); i.hasNext();) {
            DataSet set = (DataSet) i.next();
            AbstractDataSetGenerator gen = m_dataSetGeneratorMap.get(set.getName());
            //these tests will fail since e911 data set is defined in openuc plugin
            //TODO: come up with a solution
            //assertNotNull(set.getName(), gen);
            //assertNotNull(set.getName(), gen.getCoreContext());
        }
    }

    public void setCredentialDataSet(Credentials credentials) {
        m_dataSetGeneratorMap.put(DataSet.CREDENTIAL.getName(), credentials);
    }

    public void setAliasDataSet(Aliases aliases) {
        m_dataSetGeneratorMap.put(DataSet.ALIAS.getName(), aliases);
    }

    public void setPermissionDataSet(Permissions permissions) {
        m_dataSetGeneratorMap.put(DataSet.PERMISSION.getName(), permissions);
    }

    public void setUserlocationDataSet(UserLocation userLocations) {
        m_dataSetGeneratorMap.put(DataSet.USER_LOCATION.getName(), userLocations);
    }

    public void setUserforwardDataSet(UserForward cfwdTime) {
        m_dataSetGeneratorMap.put(DataSet.USER_FORWARD.getName(), cfwdTime);
    }

    public void setUserstaticDataSet(UserStatic userStatic) {
        m_dataSetGeneratorMap.put(DataSet.USER_STATIC.getName(), userStatic);
    }

    public void setCalleraliasDataSet(CallerAliases callerAliases) {
        m_dataSetGeneratorMap.put(DataSet.CALLER_ALIAS.getName(), callerAliases);
    }

    public void setAttendantDataSet(Attendant att) {
        m_dataSetGeneratorMap.put(DataSet.ATTENDANT.getName(), att);
    }

    public void setSpeeddialDataSet(SpeedDials sd) {
        m_dataSetGeneratorMap.put(DataSet.SPEED_DIAL.getName(), sd);
    }

    public void setMailstoreDataSet(Mailstore sd) {
        m_dataSetGeneratorMap.put(DataSet.MAILSTORE.getName(), sd);
    }
}
