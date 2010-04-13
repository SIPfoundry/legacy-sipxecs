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

import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

public class UploadManagerImpl extends SipxHibernateDaoSupport<Upload> implements BeanFactoryAware, UploadManager {
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
        String uploadName = upload.getName();
        if (upload.isNew() && isUploadNameUsed(uploadName)) {
            throw new UserException("&error.duplicatedUploadName", uploadName);
        }
        saveBeanWithSettings(upload);
    }

    public void deleteUpload(Upload upload) {
        upload.remove();
        deleteBeanWithSettings(upload);
    }

    public void deleteUploads(Collection<Integer> uploadIds) {
        Upload[] uploads = DaoUtils.loadBeansArrayByIds(this, Upload.class, uploadIds);
        for (Upload upload : uploads) {
            deleteUpload(upload);
        }
    }

    public Collection<Upload> getUpload() {
        return getHibernateTemplate().findByNamedQuery("upload");
    }

    public boolean isActiveUploadById(UploadSpecification spec) {
        return (getActiveUpload(spec).size() > 0);
    }

    public boolean isUploadNameUsed(String name) {
        return (getUploadName(name).size() > 0);
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

    private List<Upload> getUploadName(String name) {
        List<Upload> existing = getHibernateTemplate().findByNamedQueryAndNamedParam("uploadName", "name", name);
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
        UploadSpecification spec = upload.getSpecification();
        List<Upload> existing = getActiveUpload(spec);

        // check if this is a managed device type
        if (spec.getManaged()) {
            // should never happen
            if (existing.size() > 1) {
                throw new AlreadyDeployedException(existing.size(), spec.getLabel());
            }
            if (existing.size() == 1) {
                Upload existingUpload = existing.get(0);
                if (!existingUpload.getId().equals(upload.getId())) {
                    throw new AlreadyDeployedException(existingUpload.getName(), spec.getLabel());
                }
            }
        }

        // ensure upload is saved first. Allows it to create directory identifier
        if (upload.isNew()) {
            saveUpload(upload);
        }
        upload.deploy();
        saveUpload(upload);
    }

    public void undeploy(UploadSpecification spec) {
        List<Upload> existing = getActiveUpload(spec);

        // check if this is a managed device type
        if (spec.getManaged()) {
            // should never happen
            if (existing.size() > 1) {
                throw new AlreadyDeployedException(existing.size(), spec.getLabel());
            }
            if (existing.size() == 1) {
                Upload existingUpload = existing.get(0);
                undeploy(existingUpload);
            }
        }
    }

    public void undeploy(Upload upload) {
        upload.undeploy();
        saveUpload(upload);
    }

    static class AlreadyDeployedException extends UserException {
        private static final String ERROR = "You can only have one set of files of type \"{1}\" deployed at a time.";

        // FIXME: localize
        AlreadyDeployedException(String name, String label) {
            super("You must undeploy \"{0}\" before you can deploy these files. " + ERROR, name, label);
        }

        AlreadyDeployedException(int size, String label) {
            super("There are already {0} files sets of type \"{1}\" deployed. " + ERROR, size, label);
        }

    }

}
