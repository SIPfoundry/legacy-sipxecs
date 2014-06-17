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

import java.text.DateFormat;
import java.util.ArrayList;
import java.util.Collection;
import java.util.LinkedList;
import java.util.List;
import java.util.Locale;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlElementWrapper;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlType;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.ServiceStatus;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.job.Job;

@XmlRootElement(name = "Server")
@XmlType(propOrder = {
        "id", "host", "ip", "primary", "registered", "description"
        })
public class ServerBean {
    private int m_id;
    private String m_host;
    private String m_ip;
    private boolean m_primary;
    private boolean m_registered;
    private String m_description;

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        m_id = id;
    }

    public String getHost() {
        return m_host;
    }

    public void setHost(String host) {
        m_host = host;
    }

    public String getIp() {
        return m_ip;
    }

    public void setIp(String ip) {
        m_ip = ip;
    }

    public boolean isPrimary() {
        return m_primary;
    }

    public void setPrimary(boolean primary) {
        m_primary = primary;
    }

    public boolean isRegistered() {
        return m_registered;
    }

    public void setRegistered(boolean status) {
        m_registered = status;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public static ServerBean convertLocation(Location location, boolean status) {
        if (location == null) {
            return null;
        }
        ServerBean bean = new ServerBean();
        bean.setId(location.getId());
        bean.setHost(location.getFqdn());
        bean.setIp(location.getAddress());
        bean.setPrimary(location.isPrimary());
        bean.setRegistered(status);
        bean.setDescription(location.getName());
        return bean;
    }

    public static Location convertToLocation(Location location, ServerBean server) {
        location.setFqdn(server.getHost());
        location.setAddress(server.getIp());
        location.setName(server.getDescription());
        return location;
    }

    @XmlRootElement(name = "Service")
    @XmlType(propOrder = {
            "serviceId", "status"
            })
    public static class ServiceBean {
        private String m_serviceId;
        private String m_status;

        public void setServiceId(String service) {
            m_serviceId = service;
        }

        public String getServiceId() {
            return m_serviceId;
        }

        public void setStatus(String status) {
            m_status = status;
        }

        public String getStatus() {
            return m_status;
        }

        public static ServiceBean convertService(ServiceStatus service) {
            ServiceBean bean = new ServiceBean();
            bean.setServiceId(service.getServiceBeanId());
            bean.setStatus(service.getStatus().name());
            return bean;
        }

        public static List<ServiceBean> buildServiceList(List<ServiceStatus> services) {
            List<ServiceBean> servicesList = new LinkedList<ServiceBean>();
            for (ServiceStatus service : services) {
                servicesList.add(convertService(service));
            }
            if (servicesList.size() > 0) {
                return servicesList;
            }
            return null;
        }
    }

    @XmlRootElement(name = "Job")
    @XmlType(propOrder = {
            "name", "status", "start", "end", "server", "errMsg"
            })
    public static class JobBean {
        private String m_name;
        private String m_status;
        private String m_errMsg;
        private String m_start;
        private String m_end;
        private ServerBean m_server;

        public void setName(String name) {
            m_name = name;
        }

        public String getName() {
            return m_name;
        }

        public void setStatus(String status) {
            m_status = status;
        }

        public String getStatus() {
            return m_status;
        }

        public void setErrMsg(String msg) {
            m_errMsg = msg;
        }

        public String getErrMsg() {
            return m_errMsg;
        }

        public void setStart(String start) {
            m_start = start;
        }

        public String getStart() {
            return m_start;
        }

        public void setEnd(String end) {
            m_end = end;
        }

        public String getEnd() {
            return m_end;
        }

        public void setServer(ServerBean server) {
            m_server = server;
        }

        public ServerBean getServer() {
            return m_server;
        }

        public static JobBean convertJob(Job job, Locale locale) {
            JobBean bean = new JobBean();
            bean.setName(job.getName());
            bean.setStatus(job.getStatus().name());
            bean.setErrMsg(job.getErrorMsg());
            DateFormat dateFormat = DateFormat.getDateTimeInstance(DateFormat.SHORT, DateFormat.SHORT, locale);
            bean.setStart(dateFormat.format(job.getStart()));
            bean.setEnd(dateFormat.format(job.getStop()));
            bean.setServer(ServerBean.convertLocation(job.getLocation(), true));
            return bean;
        }

        public static List<JobBean> buildJobList(List<Job> jobs, Locale locale) {
            List<JobBean> jobList = new LinkedList<JobBean>();
            for (Job job : jobs) {
                jobList.add(convertJob(job, locale));
            }
            if (jobList.size() > 0) {
                return jobList;
            }
            return null;
        }
    }

    @XmlRootElement(name = "Feature")
    @XmlType(propOrder = {
            "name", "bundle", "type", "enabled"
            })
    public static class FeatureBean {
        private String m_name;
        private String m_bundle;
        private String m_type;
        private boolean m_enabled;

        public void setName(String name) {
            m_name = name;
        }

        public String getName() {
            return m_name;
        }

        public void setBundle(String bundle) {
            m_bundle = bundle;
        }

        public String getBundle() {
            return m_bundle;
        }

        public void setType(String type) {
            m_type = type;
        }

        public String getType() {
            return m_type;
        }

        public void setEnabled(boolean status) {
            m_enabled = status;
        }

        public boolean getEnabled() {
            return m_enabled;
        }

        public static FeatureBean convertFeature(Feature feature, boolean status, String bundle, String type) {
            FeatureBean bean = new FeatureBean();
            bean.setName(feature.getId());
            bean.setBundle(bundle);
            bean.setType(type);
            bean.setEnabled(status);
            return bean;
        }

    }

    @XmlRootElement(name = "Bundle")
    @XmlType(propOrder = {
            "name", "globalFeatures", "locationFeatures"
            })
    public static class BundleBean {
        private String m_name;
        private List<String> m_globalFeatures;
        private List<String> m_locationFeatures;

        public void setName(String name) {
            m_name = name;
        }

        public String getName() {
            return m_name;
        }

        public void setGlobalFeatures(List<String> features) {
            m_globalFeatures = features;
        }

        @XmlElementWrapper(name = "GlobalFeatures")
        @XmlElement(name = "Feature")
        public List<String> getGlobalFeatures() {
            if (m_globalFeatures == null) {
                m_globalFeatures = new ArrayList<String>();
            }
            return m_globalFeatures;
        }

        public void setLocationFeatures(List<String> features) {
            m_locationFeatures = features;
        }

        @XmlElementWrapper(name = "LocationFeatures")
        @XmlElement(name = "Feature")
        public List<String> getLocationFeatures() {
            if (m_locationFeatures == null) {
                m_locationFeatures = new ArrayList<String>();
            }
            return m_locationFeatures;
        }

        public static BundleBean convertBundle(Bundle bundle) {
            BundleBean bean = new BundleBean();
            bean.setName(bundle.getId());
            bean.setGlobalFeatures(buildFeatureList(bundle.getGlobalFeatures()));
            bean.setLocationFeatures(buildFeatureList(bundle.getLocationFeatures()));
            return bean;
        }

        public static List<String> buildFeatureList(Collection<? extends Feature> features) {
            List<String> featuresList = new LinkedList<String>();
            for (Feature feature : features) {
                featuresList.add(feature.getId());
            }
            return featuresList;
        }

        public static List<BundleBean> buildBundleList(List<Bundle> bundles) {
            List<BundleBean> bundleList = new LinkedList<BundleBean>();
            for (Bundle bundle : bundles) {
                bundleList.add(convertBundle(bundle));
            }
            if (bundleList.size() > 0) {
                return bundleList;
            }
            return null;
        }
    }

    @XmlRootElement(name = "Services")
    public static class ServiceList {

        private List<ServiceBean> m_services;

        public void setServices(List<ServiceBean> services) {
            m_services = services;
        }

        @XmlElement(name = "Service")
        public List<ServiceBean> getServices() {
            if (m_services == null) {
                m_services = new ArrayList<ServiceBean>();
            }
            return m_services;
        }

        public static ServiceList convertServiceList(List<ServiceStatus> services) {
            ServiceList list = new ServiceList();
            list.setServices(ServiceBean.buildServiceList(services));
            return list;
        }
    }

    @XmlRootElement(name = "Jobs")
    public static class JobList {

        private List<JobBean> m_jobs;

        public void setJobs(List<JobBean> jobs) {
            m_jobs = jobs;
        }

        @XmlElement(name = "Job")
        public List<JobBean> getJobs() {
            if (m_jobs == null) {
                m_jobs = new ArrayList<JobBean>();
            }
            return m_jobs;
        }

        public static JobList convertJobList(List<Job> jobs, Locale locale) {
            JobList list = new JobList();
            list.setJobs(JobBean.buildJobList(jobs, locale));
            return list;
        }
    }

    @XmlRootElement(name = "Bundles")
    public static class BundleList {

        private List<BundleBean> m_bundles;

        public void setBundles(List<BundleBean> bundles) {
            m_bundles = bundles;
        }

        @XmlElement(name = "Bundle")
        public List<BundleBean> getBundles() {
            if (m_bundles == null) {
                m_bundles = new ArrayList<BundleBean>();
            }
            return m_bundles;
        }

        public static BundleList convertBundleList(List<Bundle> bundles) {
            BundleList list = new BundleList();
            list.setBundles(BundleBean.buildBundleList(bundles));
            return list;
        }
    }

    @XmlRootElement(name = "Features")
    public static class FeatureList {

        private List<FeatureBean> m_features;

        public void setFeatures(List<FeatureBean> features) {
            m_features = features;
        }

        public List<FeatureBean> getFeatures() {
            if (m_features == null) {
                m_features = new ArrayList<FeatureBean>();
            }
            return m_features;
        }

        public static FeatureList buildFeatureList(List<FeatureBean> features) {
            FeatureList list = new FeatureList();
            list.setFeatures(features);
            return list;
        }
    }
}
