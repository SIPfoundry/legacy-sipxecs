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

import java.io.File;
import java.util.Collection;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.FileSetting;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

public class UploadManagerImpl extends SipxHibernateDaoSupport<Upload> implements BeanFactoryAware, UploadManager {
    private static final Log LOG = LogFactory.getLog(UploadManagerImpl.class);

    private static final String NAME = "name";
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
            throw new AlreadyDeployedException(uploadName);
        }
        if (!upload.isNew() && isExistingUploadNameUsed(upload.getName(), upload.getId())) {
            throw new AlreadyDeployedException(uploadName);
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
            getDaoEventPublisher().publishDelete(upload);
            deleteUpload(upload);
        }
    }

    public Collection<Upload> getUpload() {
        return getHibernateTemplate().findByNamedQuery("upload");
    }

    public void clearMissingUploads(Collection<Upload> uploads) {
        Collection<Setting> settings = null;
        String fileName = null;
        File uploadedFile = null;
        for (Upload upload : uploads) {
            boolean fileDbSynch = true;
            Setting set = upload.getSettings();
            for (Setting set2 : set.getValues()) {
                settings = set2.getValues();
                for (Setting settingObj : settings) {
                    if (settingObj.getType() instanceof FileSetting) {
                        fileName = settingObj.getValue();
                        if (fileName != null) {
                            uploadedFile = new File(
                                    upload.getUploadDirectory() + File.separatorChar + settingObj.getValue());
                            if (!uploadedFile.exists()) {
                                LOG.info("Uploaded file missing: "
                                        + uploadedFile.getAbsolutePath() + " remove associated setting");
                                settingObj.setValue(null);
                                fileDbSynch = false;
                            }
                        }
                    }
                }
            }
            if (!fileDbSynch) {
                saveUpload(upload);
            }
        }

    }

    public boolean isActiveUploadById(UploadSpecification spec) {
        return (getActiveUpload(spec).size() > 0);
    }

    public boolean isUploadNameUsed(String name) {
        return (getUploadName(name).size() > 0);
    }

    public boolean isExistingUploadNameUsed(String name, int id) {
        return (getUploadNameAndId(name, id).size() > 0);
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
        List<Upload> existing = getHibernateTemplate().findByNamedQueryAndNamedParam("uploadName", NAME, name);
        return existing;
    }

    private List<Upload> getUploadNameAndId(String name, int id) {
        List<Upload> existing = getHibernateTemplate().findByNamedQueryAndNamedParam(
                "uploadNameAndId", new String[]{NAME, "id"}, new Object[]{name, id});
        return existing;
    }

    public Upload newUpload(UploadSpecification specification) {
        Upload upload = (Upload) m_beanFactory.getBean(specification.getBeanId());
        upload.setSpecificationId(specification.getSpecificationId());
        return upload;
    }

    public void clear() {
        removeAll(Upload.class);
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
        getDaoEventPublisher().publishSave(upload);
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
        getDaoEventPublisher().publishSave(upload);
    }

    static class AlreadyDeployedException extends UserException {
        private static final String ERROR_ALREADY_DEPLOYED_NAME = "&error.alreadyDeployedName";
        private static final String ERROR_ALREADY_DEPLOYED_SIZE = "&error.alreadyDeployedSize";
        private static final String ERROR_ALREADY_DEPLOYED_UPLOAD_NAME = "&error.duplicatedUploadName";

        AlreadyDeployedException(String name, String label) {
            super(ERROR_ALREADY_DEPLOYED_NAME, name, label);
        }

        AlreadyDeployedException(int size, String label) {
            super(ERROR_ALREADY_DEPLOYED_SIZE, size, label);
        }

        AlreadyDeployedException(String name) {
            super(ERROR_ALREADY_DEPLOYED_UPLOAD_NAME, name);
        }
    }
}
