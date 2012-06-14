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

-module(spx_agentconfig_loader).
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
-define(AGENT_CONFIG_TYPE, <<"openacdagentconfigcommand">>).

-type conf() :: {Level :: pos_integer(), Dir :: string()}.

start() ->
    unload(none),

    ActionFun = fun get_action/1,
    LoadFun = ReloadFun = fun load/1,
    UnloadFun = fun unload/1,
    spx_autoloader:add_mod({?MODULE, ActionFun, LoadFun, UnloadFun, ReloadFun}, none).

%% Internal Functions

get_action(OldConf) ->
    case get_db_config(fun jprop_to_config/1) of
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
    case M:findOne(<<"entity">>, [{<<"type">>, ?AGENT_CONFIG_TYPE}]) of
        {ok, Prop} ->
            JPropToConfig(Prop)
    end.

jprop_to_config([]) ->
    {ok, none};
jprop_to_config([{<<"lstenbl">>, B} | _]) ->
    case B of
        true ->
            {ok, true};
        false ->
            {ok, none}
    end;
jprop_to_config([_ | T]) ->
    jprop_to_config(T).


-spec load(conf()) -> ok.
load(_) ->
    cpx_supervisor:update_conf(agent_dialplan_listener,
        #cpx_conf{id = agent_dialplan_listener,
            module_name = agent_dialplan_listener,
            start_function = start_link,
            start_args = [],
            supervisor = agent_connection_sup}).

-spec unload(any()) -> ok.
unload(_) ->
    cpx_supervisor:destroy(agent_dialplan_listener).

-ifdef(TEST).

-endif.
