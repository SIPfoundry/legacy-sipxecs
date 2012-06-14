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

-module(spx_call_queue_config).

-include_lib("OpenACD/include/queue.hrl").
-include_lib("OpenACD/include/call.hrl").

-ifdef(TEST).
-include_lib("eunit/include/eunit.hrl").
-endif.

-ifdef(TEST).
-export([reset_test_db/0]).
-define(DB, <<"imdb_test">>).
-else.
-define(DB, <<"imdb">>).
-endif.

-export([
	start/0,
	get_queue/1,
	get_queues/0,

	load_queues/0,

	get_queue_group/1,
	get_queue_groups/0,

	get_skill/1,
	get_skills/0,

	get_client/2,
	get_clients/0
]).

%%====================================================================
%% API
%%====================================================================

start() ->
	spx_db:connect(),

	cpx_hooks:set_hook(spx_get_queue, get_queue, ?MODULE, get_queue, [], 200),
	cpx_hooks:set_hook(spx_get_queues, get_queues, ?MODULE, get_queues, [], 200),

	cpx_hooks:set_hook(spx_get_queue_group, get_queue_group, ?MODULE, get_queue_group, [], 200),
	cpx_hooks:set_hook(spx_get_queue_groups, get_queue_groups, ?MODULE, get_queue_groups, [], 200),

	cpx_hooks:set_hook(spx_get_skill, get_skill, ?MODULE, get_skill, [], 200),
	cpx_hooks:set_hook(spx_get_skills, get_skills, ?MODULE, get_skills, [], 200),

	cpx_hooks:set_hook(spx_get_client, get_client, ?MODULE, get_client, [], 200),
	cpx_hooks:set_hook(spx_get_clients, get_clients, ?MODULE, get_clients, [], 200),

	ok.

get_queue(Name) ->
	case db_find_one(queue, [{<<"name">>, Name}]) of
		{ok, []} ->
			noexists;
		{ok, Props} ->
			spx_util:build_queue(Props);
		_ ->
			noexists
	end.

get_queues() ->
	case db_find(queue, []) of
		{ok, Props} ->
			{ok, [X || P <- Props, {ok, X} <- [spx_util:build_queue(P)]]};
		_ ->
			{ok, []}
	end.

load_queues() ->
	{ok, Qs} = get_queues(),
	lists:foreach(fun(Q) -> queue_manager:load_queue(Q#call_queue.name) end, Qs).

get_queue_group(Name) ->
	case db_find_one(queuegroup, [{<<"name">>, Name}]) of
		{ok, []} ->
			noexists;
		{ok, Props} ->
			spx_util:build_queue_group(Props);
		_ ->
			noexists
	end.

get_queue_groups() ->
	case db_find(queuegroup, []) of
		{ok, Props} ->
			{ok, [X || P <- Props, {ok, X} <- [spx_util:build_queue_group(P)]]};
		_ ->
			{ok, []}
	end.

get_skill(Atom) when is_atom(Atom) ->
	case db_find_one(skill, [{<<"atom">>, atom_to_binary(Atom, utf8)}]) of
		{ok, []} ->
			noexists;
		{ok, Props} ->
			spx_util:build_skill(Props);
		_ ->
			noexists
	end.

get_skills() ->
	case db_find(skill, []) of
		{ok, Props} ->
			{ok, [X || P <- Props, {ok, X} <- [spx_util:build_skill(P)]]};
		_ ->
			{ok, []}
	end.

get_client(Key, Val) when is_atom(Key)->
	Cond = case Key of
		id -> {<<"ident">>, Val};
		label -> {<<"name">>, Val}
	end,

	case db_find_one(client, [Cond]) of
		{ok, []} ->
			noexists;
		{ok, Props} ->
			spx_util:build_client(Props);
		_ ->
			noexists
	end.


get_clients() ->
	case db_find(client, []) of
		{ok, Props} ->
			{ok, [X || P <- Props, {ok, X} <- [spx_util:build_client(P)]]};
		_ ->
			{ok, []}
	end.


db_find(Type, Props) when is_atom(Type) ->
	db_find(as_type(Type), Props);
db_find(Type, Props) when is_binary(Type) ->
	db_find([{<<"type">>, Type}|Props]).
db_find(Props) when is_list(Props) ->
	DB = mongoapi:new(spx, ?DB),
	DB:find(<<"entity">>, Props,
		undefined, 0, 0).

db_find_one(Type, Props) when is_atom(Type) ->
	db_find_one(as_type(Type), Props);
db_find_one(Type, Props) when is_binary(Type) ->
	db_find_one([{<<"type">>, Type}|Props]).
db_find_one(Props) when is_list(Props) ->
	DB = mongoapi:new(spx, ?DB),
	DB:findOne(<<"entity">>, Props).

as_type(queue) -> <<"openacdqueue">>;
as_type(queuegroup) -> <<"openacdqueuegroup">>;
as_type(skill) -> <<"openacdskill">>;
as_type(client) -> <<"openacdclient">>.

-ifdef(TEST).
%%--------------------------------------------------------------------
%%% Test functions
%%--------------------------------------------------------------------


start_test_() ->
	{setup, fun() ->
		cpx_hooks:start_link(),
		spx_call_queue_config:start()
	end, [
		?_assert(has_hook(spx_get_queue, get_queue)),
		?_assert(has_hook(spx_get_queues, get_queues)),
		?_assert(has_hook(spx_get_queue_group, get_queue_group)),
		?_assert(has_hook(spx_get_queue_groups, get_queue_groups)),
		?_assert(has_hook(spx_get_skill, get_skill)),
		?_assert(has_hook(spx_get_skills, get_skills)),
		?_assert(has_hook(spx_get_client, get_client)),
		?_assert(has_hook(spx_get_clients, get_clients))
	]}.

defaults_test_() ->
	{setup, fun() ->
		meck:new(mongoapi),
		meck:expect(mongoapi, new, 2, {mongoapi, spx, <<"imdb_test">>}),
		meck:expect(mongoapi, findOne, 3, not_connected),
		meck:expect(mongoapi, find, 6, not_connected)
	end,
	fun(_) -> meck:unload(mongoapi) end,
	[?_assertEqual(noexists, spx_call_queue_config:get_queue("name")),
	?_assertEqual({ok, []}, spx_call_queue_config:get_queues()),
	?_assertEqual(noexists, spx_call_queue_config:get_queue_group("qg")),
	?_assertEqual({ok, []}, spx_call_queue_config:get_queue_groups()),
	?_assertEqual(noexists, spx_call_queue_config:get_skill(sk)),
	?_assertEqual({ok, []}, spx_call_queue_config:get_skills()),
	?_assertEqual(noexists, spx_call_queue_config:get_client(id, "id")),
	?_assertEqual({ok, []}, spx_call_queue_config:get_clients())
	]}.

integ_get_queue_test_() ->
	{setup, fun reset_test_db/0, fun stop_test_db/1, [
		?_assertMatch({ok, #call_queue{name="boozer", group="queuez"}},
			spx_call_queue_config:get_queue("boozer")),
		?_assertMatch(noexists,
			spx_call_queue_config:get_queue("missingqueue"))

	]}.

integ_get_queues_test_() ->
	{setup, fun reset_test_db/0, fun stop_test_db/1, [
		?_assertMatch({ok, [#call_queue{name="boozer", group="queuez"},
			#call_queue{name="homer", group="queuezon"}]},
			spx_call_queue_config:get_queues())
	]}.

integ_load_queues_test_() ->
	{setup, fun() ->
		reset_test_db(),
		meck:new(queue_manager)
	end, fun (_) ->
		stop_test_db(void),
		meck:unload(queue_manager)
	end,
	[
		fun() ->
			meck:expect(queue_manager, load_queue, 1, ok),

			spx_call_queue_config:load_queues(),

			?assert(meck:called(queue_manager, load_queue, ["boozer"])),
			?assert(meck:called(queue_manager, load_queue, ["homer"])),
			?assert(meck:validate(queue_manager))
		end
	]}.

integ_get_queue_group_test_() ->
	{setup, fun reset_test_db/0, fun stop_test_db/1, [
		?_assertMatch({ok, #queue_group{name="queuezon"}},
			spx_call_queue_config:get_queue_group("queuezon")),
		?_assertMatch(noexists,
			spx_call_queue_config:get_queue_group("nada"))
	]}.

integ_get_queue_groups_test_() ->
	{setup, fun reset_test_db/0, fun stop_test_db/1, [
		?_assertMatch({ok, [
			#queue_group{name="Default"},
			#queue_group{name="queuezon"},
			#queue_group{name="boozer"}
			]},
			spx_call_queue_config:get_queue_groups())
	]}.

integ_get_skills_test_() ->
	{setup, fun reset_test_db/0, fun stop_test_db/1, [
		?_assertMatch({ok, [
			#skill_rec{atom='_agent', group="Magic"},
			#skill_rec{atom='_brand', group="Magic"},
			#skill_rec{atom=english, group="Language"}]},
			spx_call_queue_config:get_skills())
	]}.

integ_get_skill_test_() ->
	{setup, fun reset_test_db/0, fun stop_test_db/1, [
		?_assertMatch({ok, #skill_rec{atom=english, group="Language"}},
			spx_call_queue_config:get_skill(english))
	]}.

integ_get_clients_test_() ->
	{setup, fun reset_test_db/0, fun stop_test_db/1, [
		?_assertMatch({ok, [
			#client{id="111", label="Ateneo"},
			#client{id="222", label="UST"}]},
			spx_call_queue_config:get_clients())
	]}.

integ_get_client_test_() ->
	{setup, fun reset_test_db/0, fun stop_test_db/1, [
		?_assertMatch({ok, #client{id="111", label="Ateneo"}},
			spx_call_queue_config:get_client(id, "111")),
		?_assertMatch(noexists,
			spx_call_queue_config:get_client(id, "333")),

		?_assertMatch({ok, #client{id="222", label="UST"}},
			spx_call_queue_config:get_client(label, "UST")),
		?_assertMatch(noexists,
			spx_call_queue_config:get_client(label, "LaSalle"))
	]}.

%% Test helpers

has_hook(Name, Hook) ->
	lists:member({Name, ?MODULE, Hook, [], 200},
		cpx_hooks:get_hooks(Hook)).

reset_test_db() ->
	PrivDir = case code:priv_dir(sipxplugin) of
		{error, _} ->
			filename:join([filename:dirname(code:which(spx_call_queue_config)),
				"..", "priv"]);
		Dir -> Dir
	end,
	Path = filename:join(PrivDir, "test_entries.json"),

	{ok, Bin} = file:read_file(Path),
	{struct, [{"entries", {array, Entries}}]} = mochijson:decode(Bin),

	% mongodb:start(),
	mongodb:singleServer(spx),
	mongodb:connect(spx),

	DB = mongoapi:new(spx,?DB),
	DB:set_encode_style(default),

	DB:dropDatabase(),
	lists:foreach(fun({struct, Props}) ->
		Id = proplists:get_value("_id", Props),
		P1 = proplists:delete("_id", Props),
		P2 = [{<<"_id">>, Id}| P1],
		DB:save("entity", P2) end,
	Entries).


stop_test_db(_) ->
	% catch mongodb:stop(),
	ok.

-endif.
