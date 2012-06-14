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

-module(spx_log_loader).
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

-define(LOG_CONFIG_TYPE, <<"openacdlogconfigcommand">>).

-type conf() :: {Level :: pos_integer(), Dir :: string()}.

start() ->
    spx_db:connect(),

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
    case M:findOne(<<"entity">>, [{<<"type">>, ?LOG_CONFIG_TYPE}]) of
        {ok, Prop} ->
            JPropToConfig(Prop)
    end.

jprop_to_config([]) ->
    {ok, none};
jprop_to_config(JProp) ->
    LogFile = spx_util:get_str(<<"lgfile">>, JProp),
    LogLevel = spx_util:get_atom(<<"lglvl">>, JProp),

    {ok, {LogLevel, LogFile}}.

-spec load(conf()) -> {ok, conf()}.
load({LogLevel, LogFile}) ->
    cpxlog:set_loglevel(LogFile, LogLevel),
    ok;
load(_) ->
    ok.

-spec unload(any()) -> ok.
unload(_) ->
    cpxlog:stop().

-ifdef(TEST).

load_test() ->
    LogLevel = debug,
    LogFile = "/opt/OpenACD/log/full.log",

    meck:new(cpxlog),
    meck:expect(cpxlog, set_loglevel, fun(_, _) -> ok end),

    ?assertEqual(ok, load({LogLevel, LogFile})),
    ?assert(meck:validate(cpxlog)),
    ?assert(meck:called(cpxlog, set_loglevel, [LogFile, LogLevel])),

    meck:unload(cpxlog).

unload_test() ->
    meck:new(cpxlog),
    meck:expect(cpxlog, stop, fun() -> ok end),

    ?assertEqual(ok, unload(void)),

    ?assert(meck:validate(cpxlog)),
    ?assert(meck:called(cpxlog, stop, [])),
    meck:unload(cpxlog).

empty_jprop_to_config_test() ->
    ?assertEqual({ok, none}, jprop_to_config([])).

good_jprop_to_config_test() ->
    JSON = <<"{
        \"_id\" : \"OpenAcdLogConfigCommand-1\",
        \"lglvl\" : \"debug\",
        \"lgfile\" : \"/opt/OpenACD/log/full.log\",
        \"type\" : \"openacdlogconfigcommand\"
    }">>,
    {struct, JProp} = mochijson2:decode(JSON),
    ?assertEqual({ok, {debug, "/opt/OpenACD/log/full.log"}}, jprop_to_config(JProp)).

get_db_config_test_() ->
    {foreach,
    fun() ->
        mongodb:start(),
        spx_db:connect(),
		M = mongoapi:new(spx, ?DB),
        M:dropDatabase(),
        M
    end,
    fun(M) ->
        M:dropDatabase()
        % mongodb:stop()
    end,
    [fun() ->
        mongoapi:new(spx, ?DB),
        F = fun([]) -> none; (_) -> something end,
        ?assertEqual(none, get_db_config(F))
    end,
    fun() ->
        JSON = <<"{\"type\" : \"openacdlogconfigcommand\"}">>,
        {struct, JProp} = mochijson2:decode(JSON),

        M = mongoapi:new(spx, ?DB),
        F = fun([]) -> none; (_) -> something end,
        M:save(<<"entity">>, JProp),
        ?assertEqual(something, get_db_config(F))
    end
    ]}.

-endif.
