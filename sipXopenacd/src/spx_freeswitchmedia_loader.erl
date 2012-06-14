%% Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
%% Contributed to SIPfoundry under a Contributor Agreement
%%
%% This software is free software; you can redistribute it and/or modify it under
%% the terms of the Affero General Public License (AGPL) as published by the
%% Free Software Foundation; either version 3 of the License, or (at your option)
%% any later version.
%%
%% This software is distributed in the hope that it will be useful, but WITHOUT
%% ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
%% FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
%% details.

-module(spx_freeswitchmedia_loader).
-author("eZuce").

-export([start/0]).

-ifdef(TEST).
-include_lib("eunit/include/eunit.hrl").
-endif.

-ifdef(TEST).
-define(DB, <<"imdb_test">>).
-else.
-define(DB, <<"imdb">>).
-endif.

-include_lib("OpenACD/include/cpx.hrl").
-define(FREESWITCH_CONFIG_TYPE, <<"freeswitchmediacommand">>).

-record(spx_fwconfig, {cnode, dialstring=""}).

start() ->
    unload(none),
    spx_db:connect(),

    ActionFun = fun get_action/1,
    LoadFun = ReloadFun = fun load/1,
    UnloadFun = fun unload/1,
    spx_autoloader:add_mod({?MODULE, ActionFun, LoadFun, UnloadFun, ReloadFun}, none).

%% Internal Functions

get_action(OldConf) ->
    case get_db_config(fun(X) -> jprop_to_config(X, #spx_fwconfig{}) end) of
        {ok, OldConf} ->
            none;
        {ok, none} ->
            {unload, none};
        {ok, Conf} ->
            case OldConf of
                none ->
                    {load, Conf};
                _ ->
                    {reload, Conf}
            end;
        {error, _Err} ->
            none
    end.

get_db_config(JPropToConfig) ->
    M = mongoapi:new(spx, ?DB),
    case M:findOne(<<"entity">>, [{<<"type">>, ?FREESWITCH_CONFIG_TYPE}]) of
        {ok, Prop} ->
            JPropToConfig(Prop)
    end.

%% { "_id" : "FreeswitchMediaCommand-1", "nd" : "freeswitch@127.0.0.1", "active" : true, "dlst" : "{ignore_early_media=true}sofia/sipxdev.chikka.ph/$1;sipx-noroute=VoiceMail;sipx-userforward=false", "uuid" : "7b86aa73-5789-4439-be38-ff41c603480f", "type" : "freeswitchmediacommand" }
jprop_to_config([], Acc) ->
    {ok, Acc};
jprop_to_config([{<<"active">>, false} | _], _) ->
    {ok, none};
jprop_to_config([{<<"nd">>, CNode} | T], Acc) ->
	jprop_to_config(T, Acc#spx_fwconfig{cnode = binary_to_atom(CNode, utf8)});
jprop_to_config([{<<"dlst">>, DialString} | T], Acc) ->
	jprop_to_config(T, Acc#spx_fwconfig{dialstring = binary_to_list(DialString)});
jprop_to_config([_ | T], Acc) ->
    jprop_to_config(T, Acc).


load(Conf) ->
	cpx_supervisor:update_conf(freeswitch_media_manager,
		#cpx_conf{
			id = freeswitch_media_manager,
			module_name = freeswitch_media_manager,
				start_function = start_link,
				start_args = [
					Conf#spx_fwconfig.cnode,
					[{h323,[]}, {iax2,[]}, {sip,[]},
					 {dialstring,
					 Conf#spx_fwconfig.dialstring}]],
				supervisor = mediamanager_sup
		}).

-spec unload(any()) -> ok.
unload(_) ->
    cpx_supervisor:destroy(freeswitch_media_manager).

-ifdef(TEST).

-endif.
