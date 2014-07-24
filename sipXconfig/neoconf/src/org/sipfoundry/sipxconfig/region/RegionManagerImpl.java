/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.region;

import static org.sipfoundry.sipxconfig.validation.Validate.ipv4AddrBlock;
import static org.sipfoundry.sipxconfig.validation.Validate.maxLen;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.springframework.dao.DuplicateKeyException;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowMapper;

public class RegionManagerImpl implements RegionManager {
    private static final String ERROR_NAME_IN_USE = "&error.nameInUse";
    private static final RowMapper<Region> REGION_MAPPER = new RowMapper<Region>() {
        @Override
        public Region mapRow(ResultSet rs, int row) throws SQLException {
            Region r = new Region();
            r.setUniqueId(rs.getInt("region_id"));
            r.setName(rs.getString("name"));
            r.setAddresses(decodeAddresses(rs.getString("addresses")));
            return r;
        }
    };
    private JdbcTemplate m_db;

    @Override
    public List<Region> getRegions() {
        return m_db.query("select * from region order by name", REGION_MAPPER);
    }

    @Override
    public void saveRegion(Region region) {
        validateRegion(region);
        String addresses = encodeAddresses(region.getAddresses());
        try {
            if (region.getId() == -1) {
                int nextId = m_db.queryForInt("select nextval('region_seq')");
                String sql = "insert into region (region_id, name, addresses) values (?, ?, ?)";
                m_db.update(sql, new Object[] {
                    nextId, region.getName(), addresses
                });
                region.setUniqueId(nextId);
            } else {
                String sql = "update region set name = ?, addresses = ? where region_id = ?";
                m_db.update(sql, new Object[] {
                    region.getName(), addresses, region.getId()
                });
            }
        } catch (DuplicateKeyException e) {
            throw new UserException(ERROR_NAME_IN_USE, region.getName());
        }
    }

    @Override
    public List<Integer> getServersByRegion(int regionId) {
        String sql = "select location_id from location where region_id=%d";
        return m_db.queryForList(String.format(sql, regionId),Integer.class);

    }

    void validateRegion(Region region) {
        if (Region.DEFAULT.getName().equals(region.getName())) {
            throw new UserException(ERROR_NAME_IN_USE, region.getName());
        }
        String[] addresses = region.getAddresses();
        if (addresses == null || addresses.length == 0) {
            throw new UserException("&error.RegionsCannotBeEmpty");
        }
        maxLen("&region.name", region.getName(), 255);
        for (String address : addresses) {
            ipv4AddrBlock("&region.address", address);
        }
    }

    static String encodeAddresses(String[] addresses) {
        return StringUtils.join(addresses, ' ');
    }

    static String[] decodeAddresses(String addresses) {
        return StringUtils.split(addresses, ' ');
    }

    @Override
    public void deleteRegion(Region region) {
        m_db.update("delete from region where region_id = ?", new Object[] {
            region.getId()
        });
    }

    public void setConfigJdbcTemplate(JdbcTemplate configJdbcTemplate) {
        m_db = configJdbcTemplate;
    }

    @Override
    public Region getRegion(int id) {
        Region region = m_db.queryForObject("select * from region where region_id = ?", REGION_MAPPER, id);
        return region;
    }
}
