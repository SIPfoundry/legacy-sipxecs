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

-module(sipxplugin).

-export([start/0, start_link/0, stop/0]).

start() ->
    ensure_deps(),
    sipxplugin_supervisor:start().

start_link() ->
    ensure_deps(),
    sipxplugin_supervisor:start_link().

deps() ->
    [erlmongo, 'OpenACD'].

stop() ->
    lists:foreach(fun(Dep) -> application:stop(Dep) end),
    application:stop(sipxplugin).

ensure_deps() ->
    lists:foreach(fun(Dep) -> application:start(Dep) end).

%% Internal Functions
ensure_started(App) ->
    case application:start(App) of
        ok ->
            ok;
        {error, {already_started, _}} ->
            ok
    end.