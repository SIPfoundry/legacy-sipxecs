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

-module(spx_queue_loader).
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
-include_lib("OpenACD/include/queue.hrl").

start() ->
    spx_db:connect(),

    ActionFun = fun get_action/1,
    ReloadFun = fun reload/1,
    spx_autoloader:add_mod({?MODULE, ActionFun, undefined, undefined, ReloadFun}, none).

%% Internal Functions
get_action(_) ->
    %% Non-atomic
    D = [{N, lists:sort(Skls), R, G} || {state, _, G, N, R, _, Skls, _, _} <- [call_queue:dump(P) || {_, P} <- queue_manager:queues()]],
    M = [{Q#call_queue.name, lists:sort(Q#call_queue.skills),
        Q#call_queue.recipe, Q#call_queue.group} || Q <-
            [call_queue_config:get_merged_queue(X#call_queue.name) || X <- call_queue_config:get_queues()]],

    R = lists:foldl(fun({N, _, _, _} = E, Acc) ->
        case lists:member(E, D) of
            true -> Acc;
            _ -> [N|Acc]
        end
    end, [], M),
    case R of
        [] -> none;
        _ -> {reload, R}
    end.

-spec reload(any()) -> ok.
reload(R) ->
    lists:foreach(fun(N) -> queue_manager:load_queue(N) end, R).

-ifdef(TEST).

-endif.
