/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.api.model;

import javax.xml.bind.annotation.XmlRootElement;

import org.apache.commons.beanutils.BeanUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.branch.Branch;

@XmlRootElement(name = "Branch")
public class BranchBean extends Branch {
    private static final Log LOG = LogFactory.getLog(BranchBean.class);

    public static BranchBean convertBranch(Branch branch) {
        if (branch == null) {
            return null;
        }
        try {
            BranchBean bean = new BranchBean();
            BeanUtils.copyProperties(bean, branch);
            return bean;
        } catch (Exception ex) {
            return null;
        }
    }

    public static void convertToBranch(BranchBean branchBean, Branch branch) {
        try {
            BeanUtils.copyProperties(branch, branchBean);
        } catch (Exception e) {
            LOG.error("Cannot marshal properties");
        }
    }
}
