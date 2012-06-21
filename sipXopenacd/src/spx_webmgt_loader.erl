%% Copyright (c) 2012 eZuce, Inc. All rights reserved.
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

-module(spx_webmgt_loader).
-author("eZuce").

-export([start/0]).

-export([get_action/1, load/1, unload/1]).

-ifdef(TEST).
-include_lib("eunit/include/eunit.hrl").
-endif.

-ifdef(TEST).
-define(DB, <<"imdb_test">>).
-else.
-define(DB, <<"imdb">>).
-endif.

-include_lib("OpenACD/include/cpx.hrl").
-define(WEBMGT_CONFIG_TYPE, <<"openacdwebmgtconfigcommand">>).

-record(conf, {
    enabled = false :: boolean(),
    port = 9999 :: pos_integer(),
    ssl = false :: boolean()}).

start() ->
    unload(none),
    spx_db:connect(),

    ActionFun = fun get_action/1,
    LoadFun = ReloadFun = fun load/1,
    UnloadFun = fun unload/1,
    spx_autoloader:add_mod({?MODULE, ActionFun, LoadFun, UnloadFun, ReloadFun}, none).

%% Internal Functions

get_action(OldConf) ->
    case get_db_config() of
        {ok, OldConf} ->
            none;
        {ok, C = #conf{enabled=false}} ->
            {unload, C};
        {ok, C} ->
            case OldConf of
                none ->
                    {load, C};
                _ ->
                    {reload, C}
            end;
        {error, _Err} ->
            none
    end.

get_db_config() ->
    M = mongoapi:new(spx, ?DB),
    case M:findOne(<<"entity">>, [{<<"type">>, ?WEBMGT_CONFIG_TYPE}]) of
        {ok, []} ->
            {ok, #conf{enabled = true}};
        {ok, Props} ->
            jprop_to_config(Props, #conf{})
    end.

jprop_to_config([], Conf) ->
    {ok, Conf};
jprop_to_config([{<<"prt">>, Port} | T], Conf) when is_integer(Port) ->
    jprop_to_config(T, Conf#conf{port = Port});
jprop_to_config([{<<"enbl">>, B} | T], Conf) when is_atom(B) ->
    jprop_to_config(T, Conf#conf{enabled = B});
jprop_to_config([{<<"ssl">>, B} | T], Conf) when is_atom(B) ->
    jprop_to_config(T, Conf#conf{ssl = B});
jprop_to_config([_ | T], Conf) ->
    jprop_to_config(T, Conf).


-spec load(#conf{}) -> ok.
load(Conf) ->
    cpx_supervisor:update_conf(cpx_web_management,
        #cpx_conf{id = cpx_web_management,
            module_name = cpx_web_management,
            start_function = start_link,
            start_args = [[{ssl, Conf#conf.ssl}, {port, Conf#conf.port}]],
            supervisor = management_sup}).

-spec unload(any()) -> ok.
unload(_) ->
    cpx_supervisor:destroy(cpx_web_management).

-ifdef(TEST).

-endif.
