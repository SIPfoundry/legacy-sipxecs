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

-module(spx_agentweb_loader).
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
-define(WEBMGT_CONFIG_TYPE, <<"openacdagentwebconfigcommand">>).

-record(conf, {
    http_enabled = false :: boolean(),
    http_port = 5050 :: pos_integer(),
    https_enabled = false :: boolean(),
    https_port = 5051}).

start() ->
    %% Unload if loaded
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
        {ok, C} ->
            case not (C#conf.http_enabled or C#conf.https_enabled) of
                true ->
                    {unload, C};
                _ ->
                    case OldConf of
                        none ->
                            {load, C};
                        _ ->
                            {reload, C}
                    end
            end;
        {error, _Err} ->
            none
    end.

get_db_config() ->
    M = mongoapi:new(spx, ?DB),
    case M:findOne(<<"entity">>, [{<<"type">>, ?WEBMGT_CONFIG_TYPE}]) of
        {ok, []} ->
            {ok, #conf{http_enabled = true}};
        {ok, Props} ->
            jprop_to_config(Props, #conf{})
    end.

jprop_to_config([], Conf) ->
    {ok, Conf};
jprop_to_config([{<<"enbl">>, B} | T], Conf) when is_atom(B) ->
    jprop_to_config(T, Conf#conf{http_enabled = B});
jprop_to_config([{<<"prt">>, Port} | T], Conf) when is_integer(Port) ->
    jprop_to_config(T, Conf#conf{http_port = Port});
jprop_to_config([{<<"sslenbl">>, B} | T], Conf) when is_atom(B) ->
    jprop_to_config(T, Conf#conf{https_enabled = B});
jprop_to_config([{<<"sslprt">>, Port} | T], Conf) when is_integer(Port) ->
    jprop_to_config(T, Conf#conf{https_port = Port});
jprop_to_config([_ | T], Conf) ->
    jprop_to_config(T, Conf).


-spec load(#conf{}) -> ok.
load(Conf) ->
    cpx_supervisor:update_conf(agent_web_listener,
        #cpx_conf{id = agent_web_listener,
            module_name = agent_web_listener,
            start_function = start_link,
            start_args = [[{port, Conf#conf.http_port},
                {ssl, Conf#conf.https_enabled},
                {ssl_port, Conf#conf.https_port}]],
            supervisor = management_sup}).

-spec unload(any()) -> ok.
unload(_) ->
    cpx_supervisor:destroy(agent_web_listener).

-ifdef(TEST).

-endif.
