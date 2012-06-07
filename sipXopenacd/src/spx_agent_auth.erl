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

-module(spx_agent_auth).

-include_lib("OpenACD/include/agent.hrl").

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
	get_agents/0,
	get_agents_by_profile/1,
	get_agent/2,
	auth_agent/2,
	get_profiles/0,
	get_profile/1,
	get_releases/0
]).

%%====================================================================
%% API
%%====================================================================

start() ->
	%% Agents
	cpx_hooks:set_hook(spx_get_agents, get_agents, ?MODULE, get_agents, [], 200),
	cpx_hooks:set_hook(spx_get_agents_by_profile, get_agents_by_profile, ?MODULE, get_agents_by_profile, [], 200),
	cpx_hooks:set_hook(spx_get_agent, get_agent, ?MODULE, get_agent, [], 200),
	cpx_hooks:set_hook(spx_auth_agent, auth_agent, ?MODULE, auth_agent, [], 200),
	cpx_hooks:set_hook(spx_get_profiles, get_profiles, ?MODULE, get_profiles, [], 200),
	cpx_hooks:set_hook(spx_get_profile, get_profile, ?MODULE, get_profile, [], 200),
	cpx_hooks:set_hook(spx_get_releases, get_releases, ?MODULE, get_releases, [], 200),
	ok.

-spec(get_agents/0 :: () -> {ok, [#agent_auth{}]}).
get_agents() ->
	case catch db_find(agent, []) of
		{ok, AgentProps} ->
			{ok, [X || P <- AgentProps, {ok, X} <- [spx_util:build_agent(P)]]};
		_ ->
			{ok, []}
	end.

-spec(get_agents_by_profile/1 :: (Profile :: string()) -> {ok, [#agent_auth{}]}).
get_agents_by_profile(Profile) ->
	case catch db_find(agent, [{<<"aggrp">>, iolist_to_binary(Profile)}]) of
		{ok, AgentProps} ->
			{ok, [X || P <- AgentProps, {ok, X} <- [spx_util:build_agent(P)]]};
		_ ->
			{ok, []}
	end.

-spec(get_agent/2 :: (Key :: 'id' | 'login', Value :: string()) -> {ok, #agent_auth{}} | none).
get_agent(login, Login) ->
	case catch db_find_one(agent, [{<<"name">>, Login}]) of
		{ok, []} -> none;
		{ok, P} -> spx_util:build_agent(P);
		_ -> none
	end;
get_agent(id, ID) ->
	case catch db_find_one(agent, [{<<"_id">>, ID}]) of
		{ok, []} -> none;
		{ok, P} -> spx_util:build_agent(P);
		_ -> none
	end.

-type(profile_name() :: string()).
-spec(auth_agent/2 :: (Username :: string(), Password :: string()) -> {ok, 'deny'} | {ok, {'allow', string(), skill_list(), security_level(), profile_name()}} | pass).
auth_agent(Username, Password) ->
	case catch db_find_one(agent, [{<<"name">>, Username}]) of
		{ok, []} -> pass;
		{ok, P} ->
			%UsernameBin = list_to_binary(Username),
			PasswordBin = list_to_binary(Password),
			%Realm = proplists:get_value(<<"rlm">>, P, <<>>),

			%DigestBin = crypto:md5(<<UsernameBin/binary, $:, Realm/binary, $:, PasswordBin/binary>>),
			%DigestHexBin = iolist_to_binary([io_lib:format("~2.16.0b", [C]) || <<C>> <= DigestBin]),

			PntkHexBin = proplists:get_value(<<"pntk">>, P, <<>>),
			case PntkHexBin of
				PasswordBin ->
					{ok, Auth} = spx_util:build_agent(P),
					{ok, {allow, Auth#agent_auth.id,
						Auth#agent_auth.skills,
						Auth#agent_auth.securitylevel,
						Auth#agent_auth.profile}};
				_ -> {ok, deny}
			end;
		_ -> pass
	end.

-spec(get_profiles/0 :: () -> {ok, [#agent_profile{}]}).
get_profiles() ->
	case db_find(profile, []) of
		{ok, Props} ->
			{ok, [X || P <- Props, {ok, X} <- [spx_util:build_profile(P)]]};
		_ ->
			{ok, []}
	end.

-spec(get_profile/1 :: (Name :: string() | {id, string()} | {name, string()}) -> {ok, #agent_profile{}} | 'undefined').
get_profile(Profile) ->
	case catch db_find_one(profile, [{<<"name">>, Profile}]) of
		{ok, []} -> undefined;
		{ok, P} ->
			spx_util:build_profile(P);
		_ -> undefined
	end.

-spec(get_releases/0 :: () -> {ok, [#release_opt{}]}).
get_releases() ->
	case catch db_find(release_opt, []) of
		{ok, Props} ->
			{ok, [R || P <- Props, {ok, R} <- [spx_util:build_release_opt(P)]]};
		_ ->
			{ok, []}
	end.

%% Internal functions
db_find(agent, Props) ->
	db_find(<<"openacdagent">>, Props);
db_find(profile, Props) ->
	db_find(<<"openacdagentgroup">>, Props);
db_find(release_opt, Props) ->
	db_find(<<"openacdreleasecode">>, Props);
db_find(Type, Props) when is_binary(Type) ->
	db_find([{<<"type">>, Type}|Props]).

db_find(Props) when is_list(Props) ->
	DB = mongoapi:new(spx, ?DB),
	DB:find(<<"entity">>, Props,
		undefined, 0, 0).
db_find_one(agent, Props) ->
	db_find_one(<<"openacdagent">>, Props);
db_find_one(profile, Props) ->
	db_find_one(<<"openacdagentgroup">>, Props);
db_find_one(Type, Props) when is_binary(Type) ->
	db_find_one([{<<"type">>, Type}|Props]).
db_find_one(Props) when is_list(Props) ->
	DB = mongoapi:new(spx, ?DB),
	DB:findOne(<<"entity">>, Props).


-ifdef(TEST).
%%--------------------------------------------------------------------
%%% Test functions
%%--------------------------------------------------------------------


start_test_() ->
	{setup, fun() ->
		cpx_hooks:start_link(),	
		spx_agent_auth:start()
	end, [
		?_assert(has_hook(spx_get_agents, get_agents)),
		?_assert(has_hook(spx_get_agents_by_profile, get_agents_by_profile)),
		?_assert(has_hook(spx_get_agent, get_agent)),
		?_assert(has_hook(spx_auth_agent, auth_agent)),
		?_assert(has_hook(spx_get_profiles, get_profiles)),
		?_assert(has_hook(spx_get_profile, get_profile))
	]}.

defaults_test_() ->
	{setup, fun() ->
		meck:new(mongoapi),
		meck:expect(mongoapi, new, 2, {mongoapi, spx, <<"imdb_test">>}),
		meck:expect(mongoapi, findOne, 3, not_connected),
		meck:expect(mongoapi, find, 6, not_connected)
	end,
	fun(_) -> meck:unload(mongoapi) end,
	[?_assertEqual({ok, []}, spx_agent_auth:get_agents()),
	?_assertEqual({ok, []}, spx_agent_auth:get_agents_by_profile("prof")),
	?_assertEqual(none, spx_agent_auth:get_agent(login, "login")),
	?_assertEqual(none, spx_agent_auth:get_agent(id, "id")),
	?_assertEqual(pass, spx_agent_auth:auth_agent("u", "p")),
	?_assertEqual({ok, []}, spx_agent_auth:get_profiles()),
	?_assertEqual(undefined, spx_agent_auth:get_profile("prof")),
	?_assertEqual({ok, []}, spx_agent_auth:get_releases())
	]}.

integ_get_agents_test_() ->
	{setup, fun reset_test_db/0, fun stop_test_db/1,
		[?_assertMatch({ok, [
			#agent_auth{id="agent1", login="foo", securitylevel=admin},
			#agent_auth{id="agent2", login="bar", securitylevel=agent},
			#agent_auth{id="agent3", login="baz", securitylevel=supervisor}
		]}, spx_agent_auth:get_agents())]
	}.

integ_get_agents_by_profile_test_() ->
	{setup, fun reset_test_db/0, fun stop_test_db/1,
		[?_assertMatch({ok, []}, spx_agent_auth:get_agents_by_profile("nada")),
		?_assertMatch({ok, [
			#agent_auth{id="agent1", login="foo", securitylevel=admin},
			#agent_auth{id="agent3", login="baz", securitylevel=supervisor}
		]}, spx_agent_auth:get_agents_by_profile("foobaz"))]
	}.

integ_get_agent_test_() ->
	{setup, fun reset_test_db/0, fun stop_test_db/1,
		[?_assertMatch(none, spx_agent_auth:get_agent(login, "nada")),
		?_assertMatch(none, spx_agent_auth:get_agent(id, "noone")),

		?_assertMatch({ok,
			#agent_auth{id="agent1", login="foo", securitylevel=admin}}, 
			spx_agent_auth:get_agent(login, "foo")),
		?_assertMatch({ok,
			#agent_auth{id="agent1", login="foo", securitylevel=admin}}, 
			spx_agent_auth:get_agent(id, "agent1"))
		]
	}.

integ_auth_agent_test_() ->
	{setup, fun reset_test_db/0, fun stop_test_db/1,
		[?_assertMatch(pass, spx_agent_auth:auth_agent("not", "here")),
		?_assertMatch({ok, deny}, spx_agent_auth:auth_agent("foo", "wrongpass")),
		?_assertMatch({ok, 
			{allow, "agent1", _, admin, "foobaz"}}, %% TODO fill up
			spx_agent_auth:auth_agent("foo", "foosecret"))
		]
	}.

integ_get_profiles_test_() ->
	{setup, fun reset_test_db/0, fun stop_test_db/1,
		[?_assertMatch({ok, [
			#agent_profile{id="group1"},
			#agent_profile{id="group2"}]},
			spx_agent_auth:get_profiles())
		]
	}.

integ_get_profile_test_() ->
	{setup, fun reset_test_db/0, fun stop_test_db/1,
		[?_assertMatch(undefined, spx_agent_auth:get_profile("noprofile")),
		?_assertMatch({ok, #agent_profile{id="group2"}},
			spx_agent_auth:get_profile("foobaz"))]
	}.

integ_get_releases_test_() ->
	{setup, fun reset_test_db/0, fun stop_test_db/1,
		[?_assertMatch({ok, [
			#release_opt{id="opt1", label="in a meeting", bias= -1},
			#release_opt{id="opt2", label="busy", bias=0}
			]}, spx_agent_auth:get_releases())]
	}.

%% Test helpers

has_hook(Name, Hook) ->
	lists:member({Name, ?MODULE, Hook, [], 200},
		cpx_hooks:get_hooks(Hook)).

reset_test_db() ->
	PrivDir = case code:priv_dir(sipxplugin) of
		{error, _} ->
			filename:join([filename:dirname(code:which(spx_agent_auth)),
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
