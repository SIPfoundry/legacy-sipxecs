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

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.List;

import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowMapper;

public class RegionManagerImpl implements RegionManager {
    private static final RowMapper<Region> REGION_MAPPER = new RowMapper<Region>() {
        @Override
        public Region mapRow(ResultSet rs, int row) throws SQLException {
            Region r = new Region();
            r.setUniqueId(rs.getInt("region_id"));
            r.setName(rs.getString("name"));
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
        if (region.getId() == -1) {
            int nextId = m_db.queryForInt("select nextval('region_seq')");
            String sql = "insert into region (region_id, name) values (?, ?)";
            m_db.update(sql, new Object[] {nextId, region.getName()});
            region.setUniqueId(nextId);
        } else {
            String sql = "update region set name = ? where region_id = ?";
            m_db.update(sql, new Object[] {region.getName(), region.getId()});
        }
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
