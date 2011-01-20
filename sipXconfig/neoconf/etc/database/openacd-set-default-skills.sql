insert into openacd_skill_queue (openacd_queue_id, openacd_skill_id)
	values ((select openacd_queue_id from openacd_queue where name = 'default_queue'),
			(select openacd_skill_id from openacd_skill where name = 'English'));
insert into openacd_skill_queue (openacd_queue_id, openacd_skill_id)
	values ((select openacd_queue_id from openacd_queue where name = 'default_queue'),
			(select openacd_skill_id from openacd_skill where name = 'Node'));
