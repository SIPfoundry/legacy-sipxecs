/*
 * XECS-200, Fix Call Forward External
 * remove CallForwardExternal permission settings
 * No need to touch users and group_storage tables, as it is a valid case, where 
 * value_storage_id refers to non-existing setting in setting_value table.
 */

delete from setting_value where path='permission/call-handling/ForwardCallsExternal';

