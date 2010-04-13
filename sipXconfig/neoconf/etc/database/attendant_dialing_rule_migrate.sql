-- FOR UPGRADE ONLY
UPDATE attendant_dialing_rule SET after_hours_attendant_enabled = TRUE WHERE after_hours_attendant_id IS NOT NULL;
