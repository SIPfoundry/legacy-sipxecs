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

-module(spx_integration).
-author("eZuce").

-ifdef(TEST).
-include_lib("eunit/include/eunit.hrl").
-endif.

-ifdef(TEST).
-define(DB, <<"imdb_test">>).
-else.
-define(DB, <<"imdb">>).
-endif.

-include_lib("OpenACD/include/log.hrl").
-include_lib("OpenACD/include/call.hrl").
-include_lib("OpenACD/include/queue.hrl").
-include_lib("OpenACD/include/cpx.hrl").
-include_lib("OpenACD/include/agent.hrl").

%% API
-export([
	start/0,
	start/1,
	start_link/0,
	start_link/1,
	stop/0,

	load/0,
	load_agents/0,
	load_profiles/0,
	load_clients/0,
	load_queues/0,
	load_queue_groups/0,
	load_skills/0,

	register_autoload/0
]).

%% gen_server callbacks
-export([
	init/1,
	handle_call/3,
	handle_cast/2,
	handle_info/2,
	terminate/2,
	code_change/3]).

-record(state, {queueid_ets}).

%% @doc Start linked with given options.
-spec(start_link/1 :: (Options :: any()) -> {'ok', pid()}).
start_link(Options) ->
	gen_server:start_link({local, integration}, ?MODULE, Options, []).

%% @doc start linked with default options.
-spec(start_link/0 :: () -> {'ok', pid()}).
start_link() ->
	start_link([]).

%% @doc Start unlinked.
-spec(start/1 :: (Options :: any()) -> {'ok', pid()}).
start(Options) ->
	gen_server:start({local, integration}, ?MODULE, Options, []).

%% @doc Start unlinked with default options.
-spec(start/0 :: () -> {'ok', pid()}).
start() ->
	start([]).

stop() ->
	gen_server:call(integration, stop).

register_autoload() ->
	ActionFun = fun(_) -> {reload, void} end,
	LoadFun = ReloadFun = fun (_) -> ?MODULE:load() end,
	UnloadFun = fun(_) -> ok end,
	spx_autoloader:add_mod({spx_integration, ActionFun, LoadFun, UnloadFun, ReloadFun}, void).

load_agents() ->
	%% TODO might cause errors for other integration
	gen_server:call(integration, load_agents).

load_profiles() ->
	gen_server:call(integration, load_profiles).

load_clients() ->
	gen_server:call(integration, load_clients).

load_queues() ->
	gen_server:call(integration, load_queues).

load_queue_groups() ->
	gen_server:call(integration, load_queue_groups).

load_skills() ->
	gen_server:call(integration, load_skills).

load() ->
	%load_skills(),
	%load_profiles(),
	%load_agents(),
	%load_clients(),
	%load_queue_groups(),
	load_queues().

%%====================================================================
%% gen_server callbacks
%%====================================================================

%%--------------------------------------------------------------------
%% Function: init(Args) -> {ok, State} |
%%--------------------------------------------------------------------
init(_Options) ->
	mongodb:singleServer(spx),
	ok = mongodb:connect(spx),

	%% work-arounds since plug-in is not yet loaded
	spx_agent_auth:start(),
	spx_call_queue_config:start(),
	spx_call_queue_config:load_queues(),

	{ok, #state{}}.

%--------------------------------------------------------------------
%% Function: %% handle_call(Request, From, State) -> {reply, Reply, State} |
%%                                      {reply, Reply, State, Timeout} |
%%                                      {noreply, State} |
%%                                      {noreply, State, Timeout} |
%%                                      {stop, Reason, Reply, State} |
%%                                      {stop, Reason, State}
%% Description: Handling call messages
%%--------------------------------------------------------------------
handle_call({agent_exists, Agent}, _From, #state{} = State) when is_list(Agent) ->
	DB = mongoapi:new(spx,?DB),
	case DB:count(<<"entity">>, [{<<"type">>, <<"openacdagent">>},
			{<<"name">>, Agent}]) of
		1 ->
			{reply, true, State};
		0 ->
			{reply, false, State}
	end;

handle_call({agent_auth, Name, PlainPassword, Extended}, _From, State) ->
	DB = mongoapi:new(spx, ?DB),
	Reply = case DB:findOne(<<"entity">>, [{<<"type">>, <<"openacdagent">>},
			{<<"name">>, Name}]) of
		{ok, []} ->
			destroy;
		{ok, Agent} ->
			case get_str(<<"pin">>, Agent) of
				PlainPassword ->
					Rec = props_to_agent(Agent, #agent_auth{skills=[]}),

					{ok, Rec#agent_auth.id,
						{Rec#agent_auth.profile, Rec#agent_auth.skills},
						Rec#agent_auth.securitylevel,
						Extended};
				_ ->
					deny
			end
	end,
	{reply, Reply, State};

handle_call({get_profile, Name}, _From, State) ->
	DB = mongoapi:new(spx, ?DB),
	Reply = case DB:findOne(<<"entity">>, [{<<"type">>, <<"openacdagentgroup">>},
			{<<"name">>, Name}]) of
		{ok, []} ->
			none;
		{ok, Profile} ->
			Rec = props_to_profile(Profile, #agent_profile{name = "",
				order = 10, timestamp = util:now()}),

			{ok, Rec#agent_profile.name, Rec#agent_profile.id,
				Rec#agent_profile.order, Rec#agent_profile.options,
				Rec#agent_profile.skills}
	end,
	{reply, Reply, State};

handle_call({client_exists, Key, Value}, _From, State) ->
	KeyBin = case Key of
		id -> <<"ident">>;
		label -> <<"name">>
	end,

	DB = mongoapi:new(spx, ?DB),
	Reply = case DB:count(<<"entity">>, [
				{<<"type">>, <<"OpenAcdAgent">>},
				{KeyBin, Value}]) of
		1 -> true;
		0 -> false
	end,
	{reply, Reply, State};

handle_call({get_client, Key, Value}, _From, State) ->
	KeyBin = case Key of
		id -> <<"ident">>;
		label -> <<"name">>
	end,
	DB = mongoapi:new(spx, ?DB),
	Reply = case DB:findOne(<<"entity">>, [
				{<<"type">>, <<"openacdclient">>},
				{KeyBin, Value}]) of
		{ok, []} -> none;
		{ok, Client} ->
			Rec = props_to_client(Client, #client{}),
			Id = Rec#client.id,
			Label = Rec#client.label,
			Options = [], %% TODO restore configurable client opts
			{ok, Id, Label, Options}
	end,
	{reply, Reply, State};

handle_call({get_queue, Name}, _From, State) ->
	DB = mongoapi:new(spx, ?DB),
	Reply = case DB:findOne(<<"entity">>, [
			{<<"type">>, <<"openacdqueue">>},
			{<<"name">>, Name}]) of
		{ok, []} -> none;
		{ok, Queue} ->
			Rec = props_to_queue(Queue, #call_queue{name = "", skills = []}),
			?INFO("Found: ~p~n", [Rec]),
			{ok, Rec#call_queue.name, Rec#call_queue.weight,
				Rec#call_queue.skills, Rec#call_queue.recipe,
				Rec#call_queue.hold_music, Rec#call_queue.group}
	end,
	{reply, Reply, State};

handle_call({get_queue_group, Name}, _From, State) ->
	DB = mongoapi:new(spx, ?DB),
	Reply = case DB:findOne(<<"entity">>, [
			{<<"type">>, <<"openacdqueuegroup">>},
			{<<"name">>, Name}]) of
		{ok, []} -> none;
		{ok, QueueGroup} ->
			Rec = props_to_queue_group(QueueGroup, #queue_group{name=""}),
			%% TODO Add recipe
			{ok, Rec#queue_group.name, Rec#queue_group.recipe,
				Rec#queue_group.skills, 10, false}
	end,
	{reply, Reply, State};

handle_call({get_skill, Atom}, _From, State) ->
	DB = mongoapi:new(spx, ?DB),
	Reply = case DB:findOne(<<"entity">>, [
			{<<"type">>, <<"openacdskill">>},
			{<<"atom">>, atom_to_list(Atom)}]) of
		{ok, []} -> none;
		{ok, Skill} ->
			Rec = props_to_skill(Skill, #skill_rec{}),

			{ok, Rec#skill_rec.atom,
				Rec#skill_rec.name,
				Rec#skill_rec.protected,
				Rec#skill_rec.description,
				Rec#skill_rec.group}
	end,
	{reply, Reply, State};
handle_call(stop, _From, State) ->
	?INFO("Stopping sipx integration", []),
	{stop, normal, ok, State};

handle_call(load_agents, _From, State) ->
	Type = <<"openacdagent">>,
	Now = util:now(),
	GetLoaded = fun() -> agent_auth:get_agents() end,
	PropsToRec = fun(X) -> props_to_agent(X, #agent_auth{skills=[], integrated = Now}) end,
	GetKey = fun(#agent_auth{id=ID}) -> ID end,
	Destroy = fun(#agent_auth{id=ID}) -> agent_auth:destroy(id, ID) end,
	Save = fun(Agent)-> mnesia:dirty_write(Agent) end,
	Reply = load_helper(Type, GetLoaded, PropsToRec, GetKey, Destroy, Save),
	{reply, Reply, State};

handle_call(load_clients, _From, State) ->
	Type = <<"openacdclient">>,
	Now = util:now(),

	GetLoaded = fun() -> call_queue_config:get_clients() end,
	PropsToRec = fun(X) -> props_to_client(X, #client{timestamp = Now}) end,
	GetKey = fun(#client{id=Id}) -> Id end,
	Destroy = fun(#client{id=Id}) -> call_queue_config:destroy_client(Id) end,
	Save = fun(Client)-> mnesia:dirty_write(Client) end,
	Reply = load_helper(Type, GetLoaded, PropsToRec, GetKey, Destroy, Save),
	{reply, Reply, State};

handle_call(load_profiles, _From, State) ->
	Type = <<"openacdagentgroup">>,
	Now = util:now(),

	GetLoaded = fun() -> agent_auth:get_profiles() end,
	PropsToRec = fun(X) -> props_to_profile(X, #agent_profile{name = "",
		skills = [], timestamp = Now}) end,
	GetKey = fun(#agent_profile{name=Name}) -> Name end,
	Destroy = fun(#agent_profile{name=Name}) -> agent_auth:destroy_profile(Name) end,
	Save = fun(Profile)-> mnesia:dirty_write(Profile) end,
	Reply = load_helper(Type, GetLoaded, PropsToRec, GetKey, Destroy, Save),
	{reply, Reply, State};

handle_call(load_queue_groups, _From, State) ->
	Type = <<"openacdqueuegroup">>,
	Now = util:now(),

	GetLoaded = fun() -> call_queue_config:get_queue_groups() end,
	PropsToRec = fun(X) -> props_to_queue_group(X, #queue_group{name = "",
		timestamp = Now}) end,
	GetKey = fun(#queue_group{name=Name}) -> Name end,
	Destroy = fun(#queue_group{name=Name}) -> call_queue_config:destroy_queue_group(Name) end,
	Save = fun(Group)-> mnesia:dirty_write(Group) end,
	Reply = load_helper(Type, GetLoaded, PropsToRec, GetKey, Destroy, Save),
	{reply, Reply, State};

handle_call(load_queues, _From, State) ->
	Type = <<"openacdqueue">>,
	Now = util:now(),

	%% TODO not build intermediary list
	D = [{N, lists:sort(Skls), R, G} || {state, _, G, N, R, _, Skls, _, _} <- [call_queue:dump(P) || {_, P} <- queue_manager:queues()]],
	%% TODO make atomic!
	M = [{Q#call_queue.name, lists:sort(Q#call_queue.skills),
		Q#call_queue.recipe, Q#call_queue.group} || Q <-
			[call_queue_config:get_merged_queue(X#call_queue.name) || X <- call_queue_config:get_queues()]],

	lists:foreach(fun({N, _, _, _} = E) -> case lists:member(E, D) of true -> ok; _ -> queue_manager:load_queue(N) end end, M),

	{reply, ok, State};

handle_call(load_skills, _From, State) ->
	Type = <<"openacdskill">>,
	Now = util:now(),

	GetLoaded = fun() -> call_queue_config:get_skills() end,
	PropsToRec = fun(X) -> props_to_skill(X, #skill_rec{timestamp = Now}) end,
	GetKey = fun(#skill_rec{atom=Atom}) -> Atom end,
	Destroy = fun(#skill_rec{name=Name}) -> call_queue_config:destroy_skill(Name) end,
	Save = fun(Skill)-> mnesia:dirty_write(Skill) end,
	IsProtected = fun(#skill_rec{protected=Protected}) -> Protected end,
	Reply = load_helper(Type, GetLoaded, PropsToRec, GetKey, Destroy, Save, IsProtected),
	{reply, Reply, State};

handle_call(_Request, _From, State) ->
    Reply = invalid,
    {reply, Reply, State}.

%%--------------------------------------------------------------------
%% Function: handle_cast(Msg, State) -> {noreply, State} |
%%--------------------------------------------------------------------
handle_cast(_Msg, State) ->
    {noreply, State}.

%%--------------------------------------------------------------------
%% Function: handle_info(Info, State) -> {noreply, State} |
%%--------------------------------------------------------------------
handle_info(_Info, State) ->
    {noreply, State}.

%%--------------------------------------------------------------------
%% Function: terminate(Reason, State) -> void()
%%--------------------------------------------------------------------
terminate(Reason, _State) ->
	?NOTICE("termination cause:  ~p", [Reason]),
    ok.

%%--------------------------------------------------------------------
%% Func: code_change(OldVsn, State, Extra) -> {ok, NewState}
%%--------------------------------------------------------------------
code_change(_OldVsn, State, _Extra) ->
    {ok, State}.

%%--------------------------------------------------------------------
%%% Internal functions
%%--------------------------------------------------------------------

-spec props_to_agent([{binary(), any()}], #agent_auth{}) -> #agent_auth{}.
props_to_agent([], Rec) ->
	Rec;
props_to_agent([{<<"_id">>, ID} | T], Rec) ->
	props_to_agent(T, Rec#agent_auth{id = binary_to_list(ID)});
props_to_agent([{<<"name">>, Name} | T], Rec) ->
	props_to_agent(T, Rec#agent_auth{login = binary_to_list(Name)});
props_to_agent([{<<"pin">>, Pin} | T], Rec) ->
	%% Should we be caching this?
	props_to_agent(T, Rec#agent_auth{password = util:bin_to_hexstr(erlang:md5(Pin))});
props_to_agent([{<<"skl">>, {array, Skills}} | T], Rec) ->
	OldSkills = Rec#agent_auth.skills,
	SkillAts = lists:foldl(fun (X, Acc) ->
		case catch binary_to_existing_atom(X, utf8) of
			At when is_atom(At) -> [At|Acc];
			{'EXIT', _} -> Acc
		end end, OldSkills, Skills),
	props_to_agent(T, Rec#agent_auth{skills = SkillAts});
props_to_agent([{<<"qs">>, {array, Queues}} | T], Rec) ->
	Skills = lists:foldl(fun(X, Acc) ->
		[{'_queue', binary_to_list(X)}|Acc]
	end, Rec#agent_auth.skills, Queues),
	props_to_agent(T, Rec#agent_auth{skills = Skills});
props_to_agent([{<<"clns">>, {array, Clients}} | T], Rec) ->
	Skills = lists:foldl(fun(X, Acc) ->
		[{'_brand', binary_to_list(X)}|Acc]
	end, Rec#agent_auth.skills, Clients),
	props_to_agent(T, Rec#agent_auth{skills = Skills});
props_to_agent([{<<"scrty">>, Security} | T], Rec) ->
	Atm = case Security of
		<<"ADMIN">> ->
		admin;
		<<"SUPERVISOR">> ->
		supervisor;
	_ ->
		agent
    end,
	props_to_agent(T, Rec#agent_auth{securitylevel = Atm});
props_to_agent([{<<"aggrp">>, Group} | T], Rec) ->
	props_to_agent(T, Rec#agent_auth{profile = binary_to_list(Group)});
props_to_agent([{<<"fnm">>, FirstName} | T], Rec) ->
	props_to_agent(T, Rec#agent_auth{firstname = binary_to_list(FirstName)});
props_to_agent([{<<"lnm">>, LastName} | T], Rec) ->
	props_to_agent(T, Rec#agent_auth{lastname = binary_to_list(LastName)});
props_to_agent([_ | T], Rec) ->
	props_to_agent(T, Rec).

props_to_client([], Rec) ->
	Rec;
props_to_client([{<<"ident">>, Id} | T], Rec) ->
	props_to_client(T, Rec#client{id = binary_to_list(Id)});
props_to_client([{<<"name">>, Name} | T], Rec) ->
	props_to_client(T, Rec#client{label = binary_to_list(Name)});
props_to_client([_ | T], Rec) ->
	props_to_client(T, Rec).

props_to_profile([], Rec) ->
	Rec;
props_to_profile([{<<"_id">>, Id} | T], Rec) ->
	props_to_profile(T, Rec#agent_profile{id = binary_to_list(Id)});
props_to_profile([{<<"name">>, Name} | T], Rec) ->
	props_to_profile(T, Rec#agent_profile{name = binary_to_list(Name)});
props_to_profile([{<<"skl">>, {array, Skills}} | T], Rec) ->
	OldSkills = Rec#agent_profile.skills,
	SkillAts = lists:foldl(fun (X, Acc) ->
		case catch binary_to_existing_atom(X, utf8) of
			At when is_atom(At) -> [At|Acc];
			{'EXIT', _} -> Acc
		end end, OldSkills, Skills),
	props_to_profile(T, Rec#agent_profile{skills = SkillAts});
props_to_profile([{<<"qs">>, {array, Queues}} | T], Rec) ->
	Skills = lists:foldl(fun(X, Acc) ->
		[{'_queue', binary_to_list(X)}|Acc]
	end, Rec#agent_profile.skills, Queues),
	props_to_profile(T, Rec#agent_profile{skills = Skills});
props_to_profile([{<<"clns">>, {array, Clients}} | T], Rec) ->
	Skills = lists:foldl(fun(X, Acc) ->
		[{'_brand', binary_to_list(X)}|Acc]
	end, Rec#agent_profile.skills, Clients),
	props_to_profile(T, Rec#agent_profile{skills = Skills});
props_to_profile([_ | T], Rec) ->
	props_to_profile(T, Rec).

props_to_queue_group([], Rec) ->
	Rec;
props_to_queue_group([{<<"name">>, Name} | T], Rec) ->
	props_to_queue_group(T, Rec#queue_group{name = binary_to_list(Name)});
props_to_queue_group([{<<"skl">>, {array, Skills}} | T], Rec) ->
	OldSkills = Rec#queue_group.skills,
	SkillAts = lists:foldl(fun (X, Acc) ->
		case catch binary_to_existing_atom(X, utf8) of
			At when is_atom(At) -> [At|Acc];
			{'EXIT', _} -> Acc
		end end, OldSkills, Skills),
	props_to_queue_group(T, Rec#queue_group{skills = SkillAts});
props_to_queue_group([{<<"prfl">>, {array, Profiles}} | T], Rec) ->
	Skills = lists:foldl(fun(X, Acc) ->
		[{'_profile', binary_to_list(X)}|Acc]
	end, Rec#queue_group.skills, Profiles),
	props_to_queue_group(T, Rec#queue_group{skills = Skills});
props_to_queue_group([_|T], Rec) ->
	props_to_queue_group(T, Rec).

props_to_skill([], Rec) ->
	Rec;
props_to_skill([{<<"atom">>, Atom} | T], Rec) ->
	props_to_skill(T, Rec#skill_rec{atom = binary_to_atom(Atom, utf8)});
props_to_skill([{<<"name">>, Name} | T], Rec) ->
	props_to_skill(T, Rec#skill_rec{name = binary_to_list(Name)});
props_to_skill([{<<"dscr">>, Description} | T], Rec) ->
	props_to_skill(T, Rec#skill_rec{description = binary_to_list(Description)});
props_to_skill([{<<"grpnm">>, Group} | T], Rec) ->
	props_to_skill(T, Rec#skill_rec{group = binary_to_list(Group)});
props_to_skill([_ | T], Rec) ->
	props_to_skill(T, Rec).

props_to_queue([], Rec) ->
	Rec;
props_to_queue([{<<"name">>, Name} | T], Rec) ->
	props_to_queue(T, Rec#call_queue{name = binary_to_list(Name)});
props_to_queue([{<<"wht">>, Weight} | T], Rec) ->
	props_to_queue(T, Rec#call_queue{weight = erlang:trunc(Weight)});
props_to_queue([{<<"rcps">>, {array, Recipe}} | T], Rec) ->
	props_to_queue(T, Rec#call_queue{recipe = [extract_recipe_step(X) || X <- Recipe]});
props_to_queue([{<<"qgrp">>, Group} | T], Rec) ->
	props_to_queue(T, Rec#call_queue{group = binary_to_list(Group)});
props_to_queue([{<<"hmsc">>, HoldMusic} | T], Rec) ->
	V = case HoldMusic of
		null -> undefined;
		_ -> binary_to_list(HoldMusic)
	end,
	props_to_queue(T, Rec#call_queue{hold_music = V});
props_to_queue([{<<"skl">>, {array, Skills}} | T], Rec) ->
	OldSkills = Rec#call_queue.skills,
	SkillAts = lists:foldl(fun (X, Acc) ->
		case catch binary_to_existing_atom(X, utf8) of
			At when is_atom(At) -> [At|Acc];
			{'EXIT', _} -> Acc
		end end, OldSkills, Skills),
	props_to_queue(T, Rec#call_queue{skills = SkillAts});
props_to_queue([_ | T], Rec) ->
	props_to_queue(T, Rec).

-record(spx_recipe, {conditions=[], operations=[], frequency=run_once, comment= <<>> }).
extract_recipe_step(RecipeStep) ->
	extract_recipe_step(RecipeStep, #spx_recipe{}).

%% TODO, support multiple operatiosn/actns
extract_recipe_step([], Rec) ->
	{Rec#spx_recipe.conditions, Rec#spx_recipe.operations,
		Rec#spx_recipe.frequency, Rec#spx_recipe.comment};
extract_recipe_step([{<<"cndt">>, {array, Conditions}} | T], Rec) ->
	extract_recipe_step(T, Rec#spx_recipe{conditions =
		lists:foldl(fun acc_cond/2, Rec#spx_recipe.conditions, Conditions)});
extract_recipe_step([{<<"actn">>, Operation} | T], Rec) ->
	extract_recipe_step(T, Rec#spx_recipe{operations =
		acc_op(Operation, Rec#spx_recipe.operations)});
extract_recipe_step([{<<"frq">>, <<"run_once">>} | T], Rec) ->
	extract_recipe_step(T, Rec#spx_recipe{frequency = run_once});
extract_recipe_step([{<<"frq">>, <<"run_many">>} | T], Rec) ->
	extract_recipe_step(T, Rec#spx_recipe{frequency = run_many});
extract_recipe_step([{<<"stpnm">>, Comment} | T], Rec) ->
	extract_recipe_step(T, Rec#spx_recipe{comment = Comment});
extract_recipe_step([_ | T], Rec) ->
	extract_recipe_step(T, Rec).

acc_cond(Operation, Acc) ->
	case decode_cond(Operation, {none, none, none}) of
		{ticks, _, Ticks} when is_number(Ticks) andalso Ticks > 0 ->
			[{ticks, erlang:trunc(Ticks)} | Acc];
		{eligible_agents, Comp, Num} when is_number(Num) andalso Num >= 0 ->
			[{eligible_agents, Comp, erlang:trunc(Num)} | Acc];
		{available_agents, Comp, Num} when is_number(Num) andalso Num >= 0 ->
			[{available_agents, Comp, erlang:trunc(Num)} | Acc];
		{queue_position, Comp, Num} when is_number(Num) andalso Num >= 0 ->
			[{queue_position, Comp, erlang:trunc(Num)} | Acc];
		{calls_queued, Comp, Num} when is_number(Num) andalso Num >= 0 ->
			[{calls_queued, Comp, erlang:trunc(Num)} | Acc];
		{client_calls_queued, Comp, Num} when is_number(Num) andalso Num >= 0 ->
			[{client_calls_queued, Comp, erlang:trunc(Num)} | Acc];
		{hour, Comp, Num} when is_number(Num) andalso Num >= 0 ->
			[{hour, Comp, erlang:trunc(Num)} | Acc];
		{weekday, Comp, Num} when is_number(Num) andalso Num >= 0 ->
			[{weekday, Comp, erlang:trunc(Num)} | Acc];
		{type, Comp, Type} ->
			case catch binary_to_existing_atom(Type, utf8) of
				{'EXIT', _} ->
					Acc;
				Atm ->
					[{type, Comp, Atm} | Acc]
			end;
		{client, Comp, ClientID} when is_binary(ClientID) ->
			[{client, Comp, binary_to_list(ClientID)} | Acc];
		{caller_name, Comp, RegEx} when is_binary(RegEx) ->
			[{caller_name, Comp, RegEx} | Acc];
		{caller_id, Comp, RegEx} when is_binary(RegEx) ->
			[{caller_id, Comp, RegEx} | Acc];
		_ ->
			Acc
	end.

decode_cond([], Cond) ->
	Cond;
decode_cond([{<<"cndt">>, Cndt} | T], {_, Rln, Vlu}) ->
	At = case catch binary_to_existing_atom(Cndt, utf8) of
			{'EXIT', _} ->
				none;
			Atm ->
				Atm
	end,
	decode_cond(T, {At, Rln, Vlu});
decode_cond([{<<"rln">>, <<">">>} | T], {Cndt, _, Vlu}) ->
	decode_cond(T, {Cndt, '>', Vlu});
decode_cond([{<<"rln">>, <<"<">>} | T], {Cndt, _, Vlu}) ->
	decode_cond(T, {Cndt, '<', Vlu});
decode_cond([{<<"rln">>, <<"=">>} | T], {Cndt, _, Vlu}) ->
	decode_cond(T, {Cndt, '=', Vlu});
decode_cond([{<<"rln">>, <<"!=">>} | T], {Cndt, _, Vlu}) ->
	decode_cond(T, {Cndt, '!=', Vlu});
decode_cond([{<<"vlu">>, Vlu} | T], {Cndt, Rln, _}) ->
	decode_cond(T, {Cndt, Rln, Vlu});
decode_cond([_ | T], Cond) ->
	decode_cond(T, Cond).

acc_op(Op, Acc) ->
	case decode_op(Op, {none, none}) of
		{add_skills, {array, Skills}} ->
			[{add_skills, [binary_to_atom(X, utf8) || X <- Skills]} | Acc];
		{add_skills, Skills} when is_binary(Skills) ->
			[{add_skills, split_bin_to_atoms(Skills, <<", ">>)} | Acc];
		{remove_skills, {array, Skills}} ->
			[{remove_skills, [binary_to_atom(X, utf8) || X <- Skills]} | Acc];
		{remove_skills, Skills} when is_binary(Skills) ->
			[{remove_skills, split_bin_to_atoms(Skills, <<", ">>)} | Acc];
		{set_priority, Priority} when is_number(Priority) ->
			[{set_priority, erlang:truc(Priority)} | Acc];
		{prioritize, _} ->
			[{prioritize, []} | Acc];
		{deprioritize, _} ->
			[{deprioritize, []} | Acc];
		{voicemail, _} ->
			[{voicemail, []} | Acc];
		{announce, Announcement} ->
			[{announce, binary_to_list(Announcement)} | Acc];
		_ ->
			Acc
	end.

decode_op([], Op) ->
	Op;
decode_op([{<<"action">>, Action} | T], {_, Value}) ->
	At = case catch binary_to_existing_atom(Action, utf8) of
		{'EXIT', _} ->
			none;
		V ->
			V
	end,
	decode_op(T, {At, Value});
decode_op([{<<"actionValue">>, Value} | T], {Action, _}) ->
	decode_op(T, {Action, Value});
decode_op([_ | T], Op) ->
	decode_op(T, Op).

get_str(Key, L) ->
	case proplists:get_value(Key, L) of
		undefined ->
			"";
		Bin when is_binary(Bin) ->
			%% TODO use proper encoding
			binary_to_list(Bin)
	end.

load_helper(Type, GetLoaded, PropsToRec, GetKey, Destroy, Save) ->
	load_helper(Type, GetLoaded, PropsToRec, GetKey, Destroy, Save, fun(_) -> false end).

load_helper(Type, GetLoaded, PropsToRec, GetKey, Destroy, Save, IsProtected) ->
	DB = mongoapi:new(spx, ?DB),
	%% Should ideally be atomic, but hm. not locking.
	%% Eventually it will be consistent.

	%% WARNING: loads everything into memory. TODO: Use Cursors if necessary

	%% MongoDb agents
	{ok, MdAgentProps} = DB:find(<<"entity">>, [{<<"type">>, Type}],
		undefined, 0, 0),

	MdAgents = [PropsToRec(X) || X <- MdAgentProps],
	MdAgentIdSet = gb_sets:from_list([GetKey(X) || X <- MdAgents]),
	%% TODO agents with login conflicts

	%% Mnesia agents
	MnAgents = GetLoaded(),
	MnAgentsNotInMd = lists:filter(
		fun(X) ->
			not IsProtected(X) and not gb_sets:is_element(GetKey(X), MdAgentIdSet)
		end, MnAgents),

	% Delete Agents in cache not in MongoDB
	lists:foreach(fun(Agent) -> Destroy(Agent) end,
		MnAgentsNotInMd),

	% Hoping just writing is ok. no subsequent loading.
	% probably better to agent_auth:cache/6 but need to fix password
	case erlang:fun_info(Save, arity) of
		{arity, 1} ->
			lists:foreach(fun(Agent) -> Save(Agent) end, MdAgents);
		{arity, 2} ->
			MnTree = gb_trees:from_orddict(orddict:from_list([{GetKey(X), X} || X <- MnAgents])),
			lists:foreach(fun(Agent) ->
				Mn = case gb_trees:lookup(GetKey(Agent), MnTree) of
					{value, D} -> D;
					none -> none
				end,
				Save(Agent, Mn)
			end, MdAgents)
	end.


split_bin_to_atoms(Subject, Pattern) ->
	split_bin_to_atoms0(binary:split(Subject, Pattern), Pattern, []).
split_bin_to_atoms0([<<>>], _, Acc) ->
	lists:reverse(Acc);
split_bin_to_atoms0([B], _, Acc) ->
	lists:reverse([binary_to_atom(B, utf8)|Acc]);
split_bin_to_atoms0([<<>>, Rest], Pattern, Acc) ->
	split_bin_to_atoms0(binary:split(Rest, Pattern), Pattern, Acc);
split_bin_to_atoms0([B, Rest], Pattern, Acc) ->
	split_bin_to_atoms0(binary:split(Rest, Pattern),
		Pattern, [binary_to_atom(B, utf8)|Acc]).

queue_equivalent(#call_queue{name=Name, weight=Weight, skills=SkillsA,
			recipe=RecipeA, hold_music=HoldMusic, group=Group},
		#call_queue{name=Name, weight=Weight, skills=SkillsB,
			recipe=RecipeB, hold_music=HoldMusic, group=Group}) ->
	lists:sort(SkillsA) =:= lists:sort(SkillsB) andalso recipes_equivalent(RecipeA, RecipeB);
queue_equivalent(_, _) ->
	false.


recipes_equivalent(A, B) ->
	lists:sort([sort_recipe_step(X) || X <- A]) =:= lists:sort([sort_recipe_step(X) || X <- B]).

sort_recipe_step({Conds, Ops, Run, Comment}) ->
	{lists:sort(Conds), lists:sort(Ops), Run, Comment};
sort_recipe_step(X) ->
	X.

%%%%% TESTS

-ifdef(TEST).

populate_db(DB) ->
	lists:foreach(fun(L) ->
		lists:foreach(fun(P) ->
			DB:save(<<"entity">>, P)
		end, L)
	end,
	[sample_agents(),
	sample_profiles(),
	sample_queues(),
	sample_queue_groups(),
	sample_skills(),
	sample_clients()
	]).

sample_agents() ->
	[[{<<"_id">>,<<"OpenAcdAgent1">>},
     {<<"oldnm">>,null},
     {<<"lnm">>,<<"last211">>},
     {<<"clns">>,{array,[<<"Demo Client">>]}},
     {<<"skl">>,
      {array,[<<"_agent">>,<<"_all">>, <<"_node">>, <<"_profile">>, <<"english">>]}},
     {<<"pin">>,<<"pin211">>},
     {<<"scrty">>,<<"ADMIN">>},
     {<<"aggrp">>,<<"Default">>},
     {<<"qs">>,
      {array,[<<"default_queue">>, <<"queue2">>]}},
     {<<"fnm">>,<<"first211">>},
     {<<"name">>,<<"211">>},
     {<<"uuid">>,<<"715385fe-9aa5-46e7-94e3-e8cec93235fb">>},
     {<<"type">>,<<"openacdagent">>}]].

sample_profiles() ->
	[[{<<"_id">>,<<"OpenAcdAgentGroup1">>},
     {<<"oldnm">>,null},
     {<<"skl">>,{array,[<<"german">>, <<"english">>]}},
     {<<"qs">>,{array,[<<"default_queue">>]}},
     {<<"clns">>,{array,[<<"Demo Client">>]}},
     {<<"name">>,<<"AgentGroup">>},
     {<<"uuid">>,<<"044a9dbd-5088-4024-80c3-0dec8903a3e7">>},
     {<<"type">>,<<"openacdagentgroup">>}]].

sample_queues() ->
	[[{<<"_id">>,<<"OpenAcdQueue1">>},
	     {<<"name">>,<<"vip">>},
	     {<<"oldnm">>,null},
	     {<<"prfl">>,{array,[]}},
	     {<<"wht">>,1},
	     {<<"skl">>,{array,[<<"english">>, <<"german">>]}},
	     {<<"rcps">>,{array,[]}},
	     {<<"qgrp">>,<<"Default">>},
	     {<<"uuid">>,<<"38c07d30-743c-43dd-9e3c-9133d5d5269f">>},
	     {<<"type">>,<<"openacdqueue">>}],
	 [{<<"_id">>,<<"OpenAcdQueue2">>},
	     {<<"name">>,<<"regular">>},
	     {<<"oldnm">>,null},
	     {<<"prfl">>,{array,[]}},
	     {<<"wht">>,1},
	     {<<"skl">>,{array,[<<"english">>]}},
	     {<<"rcps">>,{array,[]}},
	     {<<"qgrp">>,<<"Default">>},
	     {<<"uuid">>,<<"38c07d30-743c-43dd-9e3c-9133d5d5269f">>},
	     {<<"type">>,<<"openacdqueue">>}],
	  [{<<"_id">>,<<"OpenAcdQueue2">>},
	     {<<"name">>,<<"slowpokes">>},
	     {<<"oldnm">>,null},
	     {<<"prfl">>,{array,[]}},
	     {<<"wht">>,1},
	     {<<"skl">>,{array,[<<"english">>]}},
	     {<<"rcps">>,{array,[]}},
	     {<<"qgrp">>,<<"Default">>},
	     {<<"uuid">>,<<"38c07d30-743c-43dd-9e3c-9133d5d5269f">>},
	     {<<"type">>,<<"openacdqueue">>}]].

sample_clients() ->
	[[{<<"_id">>,<<"OpenAcdClient1">>},
	     {<<"ident">>,<<"12345">>},
	     {<<"name">>,<<"eZuce">>},
	     {<<"uuid">>,<<"7dd07176-6939-4a3f-9ad1-c82fcba07303">>},
	     {<<"type">>,<<"openacdclient">>}]].

sample_queue_groups() ->
	[[{<<"_id">>,<<"OpenAcdQueueGroup1">>},
     {<<"oldnm">>,null},
     {<<"prfl">>,{array,[<<"AgentGroup">>]}},
     {<<"skl">>,{array,[<<"german">>, <<"english">>]}},
     {<<"name">>,<<"sales">>},
     {<<"uuid">>,<<"050010cf-01c5-49ac-88b2-dcd3a4db6198">>},
     {<<"type">>,<<"openacdqueuegroup">>}]].

sample_skills() ->
	[[{<<"_id">>,<<"OpenAcdSkill1">>},
     {<<"atom">>,<<"jejemon">>},
     {<<"dscr">>,<<"Jeje Language">>},
     {<<"grpnm">>,<<"Language">>},
     {<<"name">>,<<"Jejemon">>},
     {<<"uuid">>,<<"8edde722-23da-4d93-b9ac-a1f085a252b4">>},
     {<<"type">>,<<"openacdskill">>}]].


test_setup_mongo() ->
	mnesia:clear_table(agent_auth),
	mnesia:clear_table(agent_profile),
	mnesia:clear_table(skill_rec),

	application:start(erlmongo),
	mongodb:singleServer(spx),
	mongodb:connect(spx),

	DB = mongoapi:new(spx, ?DB),
	DB:dropDatabase(),
	populate_db(DB),

	spx_integration:start_link(),

	DB.

test_setup_mnesia() ->
	mnesia:stop(),
	mnesia:delete_schema([node()]),
	mnesia:create_schema([node()]),
	mnesia:start(),
	agent_auth:build_tables(),
	call_queue_config:build_tables(),
	mnesia:clear_table(agent_auth),
	mnesia:clear_table(agent_profile),
	mnesia:clear_table(skill_rec),

	agent_auth:add_agent(#agent_auth{id = "1", login="user1",
		password=util:bin_to_hexstr(erlang:md5("pass1")),
		firstname="first1", lastname="last1",
		skills=[english], profile="profile2"}),

	agent_auth:add_agent(#agent_auth{id = "2", login="user2",
		password=util:bin_to_hexstr(erlang:md5("pass2")),
		firstname="first2", lastname="last2",
		securitylevel=admin, skills=[english], profile="profile2"}),

	agent_auth:new_profile(#agent_profile{name = "profile1", id = "1",
		timestamp = util:now()}),
	agent_auth:new_profile(#agent_profile{name = "profile2", id = "2",
		timestamp = util:now()}),
	agent_auth:new_profile(#agent_profile{name = "profile3", id = "3",
		timestamp = util:now()}),

	mnesia:dirty_write(#queue_group{name = "Default",recipe = [],skills = [],
              sort = 0,protected = true,timestamp = util:now()}),
	mnesia:dirty_write(#queue_group{name = "Marketing",recipe = [],skills = [],
              sort = 0,protected = false,timestamp = util:now()}),
    mnesia:dirty_write(#queue_group{name = "Maintenance",recipe = [],skills = [],
              sort = 0,protected = false,timestamp = util:now()}),

	mnesia:dirty_write(#skill_rec{name="English", atom=english, description="English", group = "Language"}),
	mnesia:dirty_write(#skill_rec{name="German", atom=german, description="German", group = "Language"}),

	%% Magic Skills
	mnesia:dirty_write(#skill_rec{name="Agent Name", atom='_agent', description="Magic skill that is replaced by the agent's name.", group = "Magic", protected = true}),
	mnesia:dirty_write(#skill_rec{name="Agent Profile", atom='_profile', description="Magic skill that is replaced by the agent's profile name", group = "Magic", protected = true}),
	mnesia:dirty_write(#skill_rec{name="Node", atom='_node', description="Magic skill that is replaced by the node identifier.", group = "Magic", protected = true}),
	mnesia:dirty_write(#skill_rec{name="Queue", atom='_queue', description="Magic skill replaced by a queue's name", group = "Magic", protected = true}),
	mnesia:dirty_write(#skill_rec{name="All", atom='_all', description="Magic skill to denote an agent that can answer any call regardless of other skills.", group = "Magic", protected = true}),
	mnesia:dirty_write(#skill_rec{name="Brand", atom='_brand', description="Magic skill to expand to a client's label (brand)", group="Magic", protected=true}),

	mnesia:dirty_write(#call_queue{name = "vip", skills=[english, '_node']}),
	mnesia:dirty_write(#call_queue{name = "regular"}),
	mnesia:dirty_write(#call_queue{name = "low_priority"}),


	ok.


test_cleanup_mongo(DB) ->
	DB:dropDatabase().
	%spx_integration:stop().

test_cleanup_mnesia(_) ->
	mnesia:stop(),
	mnesia:delete_schema([node()]).


agent_exists_test_() ->
	{setup, fun test_setup_mongo/0, fun test_cleanup_mongo/1,
		[?_assertEqual(true, integration:agent_exists("211")),
		?_assertEqual(false, integration:agent_exists("123"))]}.

agent_auth_test_() ->
	{setup, fun test_setup_mongo/0, fun test_cleanup_mongo/1,
	[?_assertMatch({ok, "OpenAcdAgent1", {"Default", _},
		admin, [{foo, bar}]},
		integration:agent_auth("211", "pin211", [{foo, bar}])),
	fun() ->
		{ok, _, {_, Skills}, _, _} = integration:agent_auth("211", "pin211", [{foo, bar}]),
		?assertEqual(
			lists:sort(['_agent', '_profile', '_node', '_all', english,
				{'_brand', "Demo Client"},
				{'_queue', "default_queue"}, {'_queue', "queue2"}]),
				lists:sort(Skills))
	end,
	?_assertEqual(deny,
		integration:agent_auth("211", "wrongpin", [{foo, bar}])),

	?_assertEqual(destroy,
		integration:agent_auth("nouser", "pin", [{foo, bar}]))]}.

get_profile_test_() ->
	{setup, fun test_setup_mongo/0, fun test_cleanup_mongo/1,
	[?_assertEqual(none, integration:get_profile("noprofile")),
	?_assertMatch({ok, "AgentGroup", "OpenAcdAgentGroup1", 10, [], _},
		integration:get_profile("AgentGroup")),
	fun() ->
		{ok, _, _, _, _, Skills} = integration:get_profile("AgentGroup"),
		?assertEqual(
			lists:sort([german, english, {'_queue', "default_queue"}, {'_brand', "Demo Client"}]),
			lists:sort(Skills))
	end
	]}.

get_client_test_() ->
	{setup, fun test_setup_mongo/0, fun test_cleanup_mongo/1,
	[?_assertEqual(none,
		integration:get_client(id, "noid")),
	?_assertEqual(none,
		integration:get_client(label, "nolabel")),
	?_assertEqual({ok, "12345", "eZuce", []},
		integration:get_client(id, "12345")),
	?_assertEqual({ok, "12345", "eZuce", []},
		integration:get_client(label, "eZuce"))]}.

get_queue_test_() ->
	{setup, fun test_setup_mongo/0, fun test_cleanup_mongo/1,
	[?_assertEqual(none,
		integration:get_queue("noqueue")),
	?_assertEqual({ok, "vip", 1, [german, english], [], undefined, "Default"},
		integration:get_queue("vip"))]}.

get_queue_group_test_() ->
	{setup, fun test_setup_mongo/0, fun test_cleanup_mongo/1,
	[?_assertEqual(none,
		integration:get_queue_group("noqueuegroup")),
	?_assertMatch({ok, "sales", [], _, 10, false},
		integration:get_queue_group("sales")),
	fun() ->
		{ok, _, _, Skills, _, _} = integration:get_queue_group("sales"),
		?assertEqual(lists:sort([german, english, {'_profile', "AgentGroup"}]),
			lists:sort(Skills))
	end]}.

get_skill_test_() ->
	{setup, fun test_setup_mongo/0, fun test_cleanup_mongo/1,
	[?_assertEqual(none,
		integration:get_skill(noskill)),
	?_assertEqual({ok, jejemon, "Jejemon", false, "Jeje Language", "Language"},
		integration:get_skill(jejemon))]}.

load_setup() ->
	DB = test_setup_mongo(),
	test_setup_mnesia(),
	spx_integration:start_link(),
	DB.

load_cleanup(DB) ->
	test_cleanup_mongo(DB),
	test_cleanup_mnesia(void).

load_agents_test_() ->
	{foreach, fun load_setup/0, fun load_cleanup/1,
	[fun() ->
		?assertEqual(2, length(agent_auth:get_agents())),
		spx_integration:load_agents(),
		?assertMatch([#agent_auth{
				id = "OpenAcdAgent1",
				login = "211",
				password = "3203a877dd9b46d60835e87f406e0d40",
				securitylevel = admin,
				profile = "Default",
				firstname = "first211",
				lastname = "last211"
			}], agent_auth:get_agents()),
		[Agent|_] = agent_auth:get_agents(),
		?assertEqual(
			lists:sort(['_agent', '_profile', '_node', '_all', english,
				{'_brand', "Demo Client"},
				{'_queue', "default_queue"}, {'_queue', "queue2"}]),
				lists:sort(Agent#agent_auth.skills))
	end]}.

load_clients_test_() ->
	{foreach, fun load_setup/0, fun load_cleanup/1,
	[fun() ->
		?assertEqual(2, length(call_queue_config:get_clients())),
		spx_integration:load_clients(),
		?assertMatch([#client{
				id = "12345",
				label = "eZuce"
			}], [X || X <- call_queue_config:get_clients(), X#client.id =:= "12345"])
	end]}.

load_profiles_test_() ->
	{foreach, fun load_setup/0, fun load_cleanup/1,
	[fun() ->
		?assertEqual(3, length(agent_auth:get_profiles())),
		spx_integration:load_profiles(),
		?assertMatch([#agent_profile{
				name = "AgentGroup",
				id = "OpenAcdAgentGroup1"
%				skills = ['german', 'english']
			}], agent_auth:get_profiles())
	end]}.

load_queue_groups_test_() ->
	{foreach, fun load_setup/0, fun load_cleanup/1,
	[fun() ->
		?assertEqual(3, length(call_queue_config:get_queue_groups())),
		PreloadedGroups = call_queue_config:get_queue_groups(),
		ProtectedGroups = [X || X <- PreloadedGroups, X#queue_group.protected],

		spx_integration:load_queue_groups(),
		LoadedGroups = call_queue_config:get_queue_groups(),

		?assertEqual(2, length(LoadedGroups)),
		lists:foreach(fun(Group) -> ?assert(lists:member(Group, LoadedGroups)) end,
			ProtectedGroups),

		?assertMatch([#queue_group{
			name = "sales"}],
			[X || X <- LoadedGroups, X#queue_group.name =:= "sales"])
	end]}.

load_skills_test_() ->
	{foreach, fun load_setup/0, fun load_cleanup/1,
	[fun() ->
		?assertEqual(8, length(call_queue_config:get_skills())),
		PreloadedSkills = call_queue_config:get_skills(),
		ProtectedSkills = [X || X <- PreloadedSkills, X#skill_rec.protected],

		spx_integration:load_skills(),
		LoadedSkills = call_queue_config:get_skills(),

		?assertEqual(7, length(LoadedSkills)),
		lists:foreach(fun(Skill) -> ?assert(lists:member(Skill, LoadedSkills)) end,
			ProtectedSkills),

		?assertMatch([#skill_rec{
			atom = jejemon,
			description = "Jeje Language",
			group = "Language",
			name = "Jejemon"}],
			[X || X <- LoadedSkills, X#skill_rec.atom =:= jejemon])
	end]}.

% load_queues_test_() ->
% 	{foreach, fun () ->
% 		meck:new(queue_manager),
% 		meck:expect(queue_manager, load_queue, fun(_) -> ok end),
% 		load_setup()
% 	end, fun (DB) ->
% 		load_cleanup(DB),
% 		meck:unload(queue_manager)
% 	end,
% 	[fun() ->
% 		?assertEqual(3, length(call_queue_config:get_queues())),
% 		Preloaded = call_queue_config:get_queues(),

% 		spx_integration:load_queues(),
% 		LoadedSkills = call_queue_config:get_skills(),

% 		?assertEqual(4, length(LoadedSkills)),
% 		?assert(meck:called(queue_manager, load_queue, [vip])),
% 		?assert(not meck:called(queue_manager, load_queue, [regular]))

% 		% ?assertMatch([#skill_rec{
% 		% 	atom = jejemon,
% 		% 	description = "Jeje Language",
% 		% 	group = "Language",
% 		% 	name = "Jejemon"}],
% 		% 	[X || X <- LoadedSkills, X#skill_rec.atom =:= jejemon])
% 	end]}.


% [[{<<"_id">>,<<"OpenAcdSkill1">>},
%      {<<"atom">>,<<"english">>},
%      {<<"dscr">>,<<"English Desc">>},
%      {<<"grpnm">>,<<"Language">>},
%      {<<"name">>,<<"English">>},
%      {<<"uuid">>,<<"8edde722-23da-4d93-b9ac-a1f085a252b4">>},
%      {<<"type">>,<<"openacdskill">>}]].


% [{<<"_id">>,<<"OpenAcdQueue4">>},
%      {<<"oldnm">>,<<"NewQueueName222">>},
%      {<<"prfl">>,{array,[<<"Default">>]}},
%      {<<"wht">>,5.0},
%      {<<"skl">>,
%       {array,[<<"german">>,<<"_node">>,<<"new_skill">>,
%               <<"_queue">>,<<"english">>,<<"_all">>,<<"_brand">>]}},
%      {<<"rcps">>,
%       {array,[[{<<"actn">>,
%                 [{<<"action">>,<<"prioritize">>},{<<"actionValue">>,<<>>}]},
%                {<<"cndt">>,
%                 {array,[[{<<"cndt">>,<<"ticks">>},
%                          {<<"rln">>,<<"=">>},
%                          {<<"vlu">>,5.0}],
%                         [{<<"cndt">>,<<"available_agents">>},
%                          {<<"rln">>,<<"=">>},
%                          {<<"vlu">>,5.0}],
%                         [{<<"cndt">>,<<"calls_queued">>},
%                          {<<"rln">>,<<"=">>},
%                          {<<"vlu">>,5.0}],
%                         [{<<"cndt">>,<<"queue_position">>},
%                          {<<"rln">>,<<"=">>},
%                          {<<"vlu">>,5.0}],
%                         [{<<"cndt">>,<<"client">>},
%                          {<<"rln">>,<<"=">>},
%                          {<<"vlu">>,5.0}],
%                         [{<<"cndt">>,<<"hour">>},
%                          {<<"rln">>,<<"=">>},
%                          {<<"vlu">>,5.0}],
%                         [{<<"cndt">>,<<"weekday">>},
%                          {<<"rln">>,<<"=">>},
%                          {<<"vlu">>,5.0}],
%                         [{<<"cndt">>,<<"type">>},
%                          {<<"rln">>,<<"=">>},
%                          {<<"vlu">>,5.0}],
%                         [{<<"cndt">>,<<"client_calls_queued">>},
%                          {<<"rln">>,<<"=">>},
%                          {<<"vlu">>,5.0}]]}},
%                {<<"frq">>,<<"run_once">>},
%                {<<"stpnm">>,<<"New Step">>}],
%               [{<<"actn">>,
%                 [{<<"action">>,<<"add_skills">>},
%                  {<<"actionValue">>,
%                   <<"german, aa, _node, new_skill, _queue, english, _all, _brand">>}]},
%                {<<"cndt">>,
%                 {array,[[{<<"cndt">>,<<"calls_queued">>},
%                          {<<"rln">>,<<"=">>},
%                          {<<"vlu">>,5.0}]]}},
%                {<<"frq">>,<<"run_once">>},
%                {<<"stpnm">>,<<"New Step">>}],
%               [{<<"actn">>,
%                 [{<<"action">>,<<"deprioritize">>},
%                  {<<"actionValue">>,<<>>}]},
%                {<<"cndt">>,
%                 {array,[[{<<"cndt">>,<<"ticks">>},
%                          {<<"rln">>,<<"=">>},
%                          {<<"vlu">>,5.0}]]}},
%                {<<"frq">>,<<"run_once">>},
%                {<<"stpnm">>,<<"New Step">>}],
%               [{<<"actn">>,
%                 [{<<"action">>,<<"set_priority">>},
%                  {<<"actionValue">>,<<"5">>}]},
%                {<<"cndt">>,
%                 {array,[[{<<"cndt">>,<<"ticks">>},
%                          {<<"rln">>,<<"=">>},
%                          {<<"vlu">>,5.0}]]}},
%                {<<"frq">>,<<"run_once">>},
%                {<<"stpnm">>,<<"New Step">>}],
%               [{<<"actn">>,
%                 [{<<"action">>,<<"voicemail">>},{<<"actionValue">>,<<>>}]},
%                {<<"cndt">>,
%                 {array,[[{<<"cndt">>,<<"ticks">>},
%                          {<<"rln">>,<<"=">>},
%                          {<<"vlu">>,51.0}]]}},
%                {<<"frq">>,<<"run_once">>},
%                {<<"stpnm">>,<<"New Step">>}],
%               [{<<"actn">>,
%                 [{<<"action">>,<<"announce">>},
%                  {<<"actionValue">>,<<"aaaa">>}]},
%                {<<"cndt">>,
%                 {array,[[{<<"cndt">>,<<"ticks">>},
%                          {<<"rln">>,<<"=">>},
%                          {<<"vlu">>,10.0}]]}},
%                {<<"frq">>,<<"run_once">>},
%                {<<"stpnm">>,<<"New Step">>}],
%               [{<<"actn">>,
%                 [{<<"action">>,<<"remove_skills">>},
%                  {<<"actionValue">>,
%                   <<"german, _node, new_skill, _queue, english, _all, _brand">>}]},
%                {<<"cndt">>,
%                 {array,[[{<<"cndt">>,<<"ticks">>},
%                          {<<"rln">>,<<"=">>},
%                          {<<"vlu">>,5.0}]]}},
%                {<<"frq">>,<<"run_once">>},
%                {<<"stpnm">>,<<"New Step">>}]]}},
%      {<<"qgrp">>,<<"QueueGroupNames">>},
%      {<<"name">>,<<"NewQueueName222">>},
%      {<<"uuid">>,<<"a77ea957-4921-465f-8b97-6a64415495f5">>},
%      {<<"type">>,<<"openacdqueue">>}]

-endif.
