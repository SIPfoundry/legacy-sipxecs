/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.upload;

import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

public class UploadManagerImpl extends SipxHibernateDaoSupport implements BeanFactoryAware, UploadManager {
    private ListableBeanFactory m_beanFactory;
    private ModelSource<UploadSpecification> m_specificationSource;

    /**
     * Callback that supplies the owning factory to a bean instance.
     */
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    public Upload loadUpload(Integer uploadId) {
        return (Upload) getHibernateTemplate().load(Upload.class, uploadId);
    }

    public void saveUpload(Upload upload) {
        saveBeanWithSettings(upload);
    }

    public void deleteUpload(Upload upload) {
        upload.remove();
        deleteBeanWithSettings(upload);
    }
    
    public Collection<Upload> getUpload() {
        return getHibernateTemplate().findByNamedQuery("upload");
    }
    
    public boolean isActiveUploadById(UploadSpecification spec) {
        return (getActiveUpload(spec).size() > 1);
    }
        
    
    public void setSpecificationSource(ModelSource<UploadSpecification> specificationSource) {
        m_specificationSource = specificationSource;
    }

    public UploadSpecification getSpecification(String specId) {
        return m_specificationSource.getModel(specId);
    }
    
    private List<Upload> getActiveUpload(UploadSpecification spec) {
        List<Upload> existing = getHibernateTemplate().findByNamedQueryAndNamedParam(
                "deployedUploadBySpecification", "spec", spec.getSpecificationId());
        return existing;
    }
        
    public Upload newUpload(UploadSpecification specification) {       
        Upload upload = (Upload) m_beanFactory.getBean(specification.getBeanId());
        upload.setSpecificationId(specification.getSpecificationId());
        return upload;
    }

    public void clear() {
        getHibernateTemplate().deleteAll(getUpload());
    }

    public void deploy(Upload upload) {
        // ensure upload is saved first.  Allows it to create directory identifier 
        if (upload.isNew()) {
            saveUpload(upload);
        }
        
        UploadSpecification spec = upload.getSpecification();
        List<Upload> existing = getActiveUpload(spec);
        // should never happen
        if (existing.size() > 1) {
            throw new AlreadyDeployedException("There are already " + existing.size() 
                    + " files sets of type  \"" + spec.getLabel() + "\" deployed.  You can only have "
                    + " one set of files of this type deployed at a time");                
        }
        if (existing.size() == 1) {
            Upload existingUpload = existing.get(0);
            if (!existingUpload.getId().equals(upload.getId())) {
                throw new AlreadyDeployedException("You must undeploy \"" +  existingUpload.getName()
                        + "\" before you can deploy these files.  You can only have one set of files of type \""
                        + spec.getLabel() + "\" deployed at a time.");
            }
        }
        
        upload.deploy();        
        saveUpload(upload);
    }

    public void undeploy(Upload upload) {
        upload.undeploy();
        saveUpload(upload);
    }

    static class AlreadyDeployedException extends UserException {
        AlreadyDeployedException(String msg) {
            super(msg);
        }
    }
    
}
