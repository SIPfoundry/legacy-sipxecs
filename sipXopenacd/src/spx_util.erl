%% Copyright (c) 2011 / 2012 eZuce, Inc. All rights reserved.
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

-module(spx_util).
-author("eZuce").

-include_lib("OpenACD/include/agent.hrl").
-include_lib("OpenACD/include/queue.hrl").
-include_lib("OpenACD/include/call.hrl").

-include_lib("OpenACD/include/log.hrl").

-ifdef(TEST).
-include_lib("eunit/include/eunit.hrl").
-endif.

-record(spx_recipe, {conditions=[], operations=[], frequency=run_once, comment= <<>> }).

-export([
	get_str/2,
	get_atom/2,
	get_bin/2,
	get_str_to_int/2,
	get_int/2,

	build_agent/1,
	build_profile/1,
	build_release_opt/1,
	build_queue/1,
	build_queue_group/1,
	build_skill/1,
	build_client/1
]).

-ifdef(TEST).
-export([build_recipe/1]).
-endif.

get_str(Key, L) ->
	case proplists:get_value(Key, L) of
		undefined ->
			"";
		Bin when is_binary(Bin) ->
			%% TODO use proper encoding
			binary_to_list(Bin)
	end.

get_atom(Key, L) ->
	case proplists:get_value(Key, L) of
		undefined ->
			undefined;
		Bin when is_binary(Bin) ->
			%% TODO must use list_to_existing_atom
			binary_to_atom(Bin, utf8)
	end.

get_bin(Key, L) ->
	proplists:get_value(Key, L, <<>>).

get_str_to_int(Key, L) ->
	case proplists:get_value(Key, L) of
		undefined ->
			0;
		Bin when is_binary(Bin) ->
			try list_to_integer(binary_to_list(Bin)) of
				V -> V
			catch
				error:badarg -> 0
			end
	end.

get_int(Key, L) ->
	case proplists:get_value(Key, L) of
		undefined ->
			0;
		Num when is_number(Num) ->
			erlang:trunc(Num)
	end.

build_agent(Props) ->
	Agent = build_agent(Props, #agent_auth{skills=[]}),
	case Agent#agent_auth.id of
		undefined -> {error, noid};
		_ -> {ok, Agent}
	end.

build_agent([], Acc) ->
	Acc;
build_agent([{<<"_id">>, Id}|T], Acc) ->
	build_agent(T, Acc#agent_auth{id=binary_to_list(Id)});

%% Name
build_agent([{<<"name">>, Login}|T], Acc) ->
	build_agent(T, Acc#agent_auth{login=binary_to_list(Login)});

%% Skills
build_agent([{<<"skl">>, {array, Skills}}|T], Acc) ->
	OldSkills = Acc#agent_auth.skills,
	build_agent(T, Acc#agent_auth{skills=
		OldSkills ++ [binary_to_atom(X, utf8) || X <- Skills]});
build_agent([{<<"qs">>, {array, Queues}}|T], Acc) ->
	OldSkills = Acc#agent_auth.skills,
	build_agent(T, Acc#agent_auth{skills=
		OldSkills ++ [{'_queue', binary_to_list(X)} || X <- Queues]});
build_agent([{<<"clns">>, {array, Clients}}|T], Acc) ->
	OldSkills = Acc#agent_auth.skills,
	build_agent(T, Acc#agent_auth{skills=
		OldSkills ++ [{'_brand', binary_to_list(X)} || X <- Clients]});

%% Security Level
build_agent([{<<"scrty">>, <<"ADMIN">>}|T], Acc) ->
	build_agent(T, Acc#agent_auth{securitylevel = admin});
build_agent([{<<"scrty">>, <<"SUPERVISOR">>}|T], Acc) ->
	build_agent(T, Acc#agent_auth{securitylevel = supervisor});
build_agent([{<<"scrty">>, <<"AGENT">>}|T], Acc) ->
	build_agent(T, Acc#agent_auth{securitylevel = agent});

%% Profile
build_agent([{<<"aggrp">>, Profile}|T], Acc) ->
	build_agent(T, Acc#agent_auth{profile = binary_to_list(Profile)});

%% First Name
build_agent([{<<"fnm">>, FirstName}|T], Acc) ->
	build_agent(T, Acc#agent_auth{firstname = binary_to_list(FirstName)});

%% Last Name
build_agent([{<<"lnm">>, LastName}|T], Acc) ->
	build_agent(T, Acc#agent_auth{lastname = binary_to_list(LastName)});

%% Endpoint
build_agent([{<<"cnt">>, SipURI}|T], Acc) ->
	SipURIStr = binary_to_list(SipURI),
	EndpointOpts = [{type, pstn}, {data, SipURIStr}],

	FwEndpoint = {freeswitch_media, EndpointOpts},
	VmEndpoint = {freeswitch_voicemail, EndpointOpts},

	OldEndpoints = Acc#agent_auth.endpoints,
	build_agent(T, Acc#agent_auth{endpoints = [FwEndpoint, VmEndpoint|OldEndpoints]});

%% Unknown
build_agent([_Prop|T], Acc) ->
	% ?WARNING("Unkown agent_auth property: ~p", [_Prop]),
	build_agent(T, Acc).


build_profile(Props) ->
	Profile = build_profile(Props, #agent_profile{name=undefined, skills=[]}),
	case {Profile#agent_profile.id, Profile#agent_profile.name} of
		{undefined, _} -> {error, noid};
		{_, undefined} -> {error, noname};
		_ -> {ok, Profile}
	end.

%% Profile

build_profile([], Acc) ->
	Acc;
build_profile([{<<"_id">>, Id}|T], Acc) ->
	build_profile(T, Acc#agent_profile{id=binary_to_list(Id)});
build_profile([{<<"name">>, Name}|T], Acc) ->
	build_profile(T, Acc#agent_profile{name=binary_to_list(Name)});
build_profile([{<<"skl">>, {array, Skills}}|T], Acc) ->
	OldSkills = Acc#agent_profile.skills,
	build_profile(T, Acc#agent_profile{skills=
		OldSkills ++ [binary_to_atom(X, utf8) || X <- Skills]});
build_profile([{<<"qs">>, {array, Queues}}|T], Acc) ->
	OldSkills = Acc#agent_profile.skills,
	build_profile(T, Acc#agent_profile{skills=
		OldSkills ++ [{'_queue', binary_to_list(X)} || X <- Queues]});
build_profile([{<<"clns">>, {array, Clients}}|T], Acc) ->
	OldSkills = Acc#agent_profile.skills,
	build_profile(T, Acc#agent_profile{skills=
		OldSkills ++ [{'_brand', binary_to_list(X)} || X <- Clients]});
build_profile([_|T], Acc) ->
	build_profile(T, Acc).

%% Release Opt

build_release_opt(P) ->
	RelOpt = build_release_opt(P, #release_opt{}),
	case {RelOpt#release_opt.id, RelOpt#release_opt.label} of
		{undefined, _} -> {error, noid};
		{invalid, _} -> {error, invalidid};
		{_, undefined} -> {error, nolabel};
		{_, _} -> {ok, RelOpt}
	end.

build_release_opt([], Acc) ->
	Acc;
build_release_opt([{<<"_id">>, Id}|T], Acc) ->
	case catch list_to_integer(re:replace(Id, "\\D*", "", [{return, list}])) of
		Num when is_integer(Num) ->
			build_release_opt(T, Acc#release_opt{id=Num});
		_ ->
			Acc#release_opt{id=invalid}
	end;
build_release_opt([{<<"lbl">>, Label}|T], Acc) ->
	build_release_opt(T, Acc#release_opt{label=binary_to_list(Label)});
build_release_opt([{<<"bias">>, Bias}|T], Acc) when is_integer(Bias) ->
	build_release_opt(T, Acc#release_opt{bias=Bias});
build_release_opt([_|T], Acc) ->
	build_release_opt(T, Acc).

%% Queue

build_queue(P) ->
	Q = build_queue(P, #call_queue{name=undefined, skills=[]}),
	if Q#call_queue.name =:= undefined -> {error, noname};
		true -> {ok, Q}
	end.
build_queue([], Acc) ->
	Acc;
build_queue([{<<"name">>, Name}|T], Acc) ->
	build_queue(T, Acc#call_queue{name=binary_to_list(Name)});
build_queue([{<<"skl">>, {array, Skills}}|T], Acc) ->
	OldSkills = Acc#call_queue.skills,
	build_queue(T, Acc#call_queue{skills=
		OldSkills ++ [binary_to_atom(X, utf8) || X <- Skills]});
build_queue([{<<"qs">>, {array, Queues}}|T], Acc) ->
	OldSkills = Acc#call_queue.skills,
	build_queue(T, Acc#call_queue{skills=
		OldSkills ++ [{'_queue', binary_to_list(X)} || X <- Queues]});
build_queue([{<<"clns">>, {array, Clients}}|T], Acc) ->
	OldSkills = Acc#call_queue.skills,
	build_queue(T, Acc#call_queue{skills=
		OldSkills ++ [{'_brand', binary_to_list(X)} || X <- Clients]});
build_queue([{<<"wht">>, Weight}|T], Acc) when is_integer(Weight) ->
	build_queue(T, Acc#call_queue{weight=Weight});
build_queue([{<<"qgrp">>, Group}|T], Acc) when is_binary(Group) ->
	build_queue(T, Acc#call_queue{group=binary_to_list(Group)});
build_queue([{<<"rcps">>, {array, Recipe}}|T], Acc) ->
	build_queue(T, Acc#call_queue{recipe=build_recipe(Recipe)});
build_queue([_|T], Acc) ->
	build_queue(T, Acc).

%% Queue Group
build_queue_group(P) ->
	G = build_queue_group(P, #queue_group{name=undefined, skills=[]}),
	if G#queue_group.name =:= undefined -> {error, noname};
		true -> {ok, G}
	end.

build_queue_group([], Acc) ->
	Acc;
build_queue_group([{<<"name">>, Name}|T], Acc) ->
	build_queue_group(T, Acc#queue_group{name=binary_to_list(Name)});
build_queue_group([{<<"skl">>, {array, Skills}}|T], Acc) ->
	OldSkills = Acc#queue_group.skills,
	build_queue_group(T, Acc#queue_group{skills=
		OldSkills ++ [binary_to_atom(X, utf8) || X <- Skills]});
build_queue_group([{<<"qs">>, {array, Queues}}|T], Acc) ->
	OldSkills = Acc#queue_group.skills,
	build_queue_group(T, Acc#queue_group{skills=
		OldSkills ++ [{'_queue', binary_to_list(X)} || X <- Queues]});
build_queue_group([{<<"clns">>, {array, Clients}}|T], Acc) ->
	OldSkills = Acc#queue_group.skills,
	build_queue_group(T, Acc#queue_group{skills=
		OldSkills ++ [{'_brand', binary_to_list(X)} || X <- Clients]});
build_queue_group([_|T], Acc) ->
	build_queue_group(T, Acc).

%% Recipe
build_recipe(Steps) ->
	build_recipe(Steps, []).

build_recipe([], Acc) ->
	lists:reverse(Acc);
build_recipe([S|T], Acc) ->
	Step = build_recipe_step(S, #spx_recipe{}),
	build_recipe(T, [Step|Acc]).

build_recipe_step([], Acc) ->
	{Acc#spx_recipe.conditions, Acc#spx_recipe.operations,
		Acc#spx_recipe.frequency, Acc#spx_recipe.comment};
build_recipe_step([{<<"cndt">>, {array, Conds}}|T], Acc) ->
	build_recipe_step(T, Acc#spx_recipe{conditions=build_recipe_conds(Conds, [])});
build_recipe_step([{<<"actn">>, Action}|T], Acc) when is_list(Action) ->
	case proplists:get_value(<<"action">>, Action) of
		ActionBin when is_binary(ActionBin) ->
			case read_recipe_action(ActionBin, Action) of
				{ok, Op} ->
					build_recipe_step(T, Acc#spx_recipe{operations=[Op]});
				{error, Err} ->
					?WARNING("Error reading action: ~p: ~p", [Action, Err]),
					build_recipe_step(T, Acc)
			end
	end;
build_recipe_step([{<<"frq">>, <<"run_once">>}|T], Acc) ->
	build_recipe_step(T, Acc#spx_recipe{frequency=run_once});
build_recipe_step([{<<"frq">>, <<"run_many">>}|T], Acc) ->
	build_recipe_step(T, Acc#spx_recipe{frequency=run_many});
build_recipe_step([{<<"stpnm">>, StepNm}|T], Acc) when is_binary(StepNm) ->
	build_recipe_step(T, Acc#spx_recipe{comment=StepNm});
build_recipe_step([_|T], Acc) ->
	build_recipe_step(T, Acc).

build_recipe_conds([], Acc) ->
	lists:reverse(Acc);
build_recipe_conds([Cond|T], Acc) ->
	Acc1 = case read_recipe_cond(proplists:get_value(<<"cndt">>, Cond), Cond) of
		{ok, C} -> [C|Acc];
		{error, Err} ->
			?WARNING("Invalid condition: ~p - ~p", [Cond, Err]),
			Acc
	end,
	build_recipe_conds(T, Acc1).

read_recipe_cond(<<"ticks">>, P) ->
	case proplists:get_value(<<"vlu">>, P) of
		N when is_number(N) andalso N >= 0.0 ->
			T = erlang:trunc(N),
			{ok, {ticks, T}};
		_ ->
			{error, invalid_vlu}
	end;

read_recipe_cond(<<"eligible_agents">>, P) ->
	form_recipe_cond_crv(eligible_agents, P);
read_recipe_cond(<<"available_agents">>, P) ->
	form_recipe_cond_crv(available_agents, P);
read_recipe_cond(<<"queue_position">>, P) ->
	form_recipe_cond_crv(queue_position, P);
read_recipe_cond(<<"calls_queued">>, P) ->
	form_recipe_cond_crv(calls_queued, P);
read_recipe_cond(<<"client_calls_queued">>, P) ->
	form_recipe_cond_crv(client_calls_queued, P);
read_recipe_cond(<<"hour">>, P) ->
	form_recipe_cond_crv(hour, P);
read_recipe_cond(<<"weekday">>, P) ->
	form_recipe_cond_crv(weekday, P);
read_recipe_cond(<<"client">>, P) ->
	form_recipe_cond_str_crv(client, P);
read_recipe_cond(<<"caller_name">>, P) ->
	form_recipe_cond_str_crv(caller_name, P);
read_recipe_cond(<<"caller_id">>, P) ->
	form_recipe_cond_str_crv(caller_id, P);
read_recipe_cond(<<"type">>, P) ->
	form_recipe_cond_atm_crv(type, P).

form_recipe_cond_crv(Atm, P) ->
	T = case proplists:get_value(<<"vlu">>, P) of
		N when is_number(N) andalso N >= 0.0 ->
			erlang:trunc(N);
		_ ->
			undefined
	end,
	Rel = case proplists:get_value(<<"rln">>, P) of
		<<">">> -> '>';
		<<"<">> -> '<';
		<<"=">> -> '=';
		% <<"!=">> -> '!=';
		_ -> undefined
	end,
	
	case {T, Rel} of
		{undefined, _} -> {error, invalid_vlu};
		{_, undefined} -> {error, invalid_rln};
		{_, _} -> {ok, {Atm, Rel, T}}
	end.

form_recipe_cond_str_crv(Atm, P) ->
	T = case proplists:get_value(<<"vlu">>, P) of
		S when is_binary(S) ->
			binary_to_list(S);
		_ ->
			undefined
	end,
	Rel = case proplists:get_value(<<"rln">>, P) of
		<<"=">> -> '=';
		<<"!=">> -> '!=';
		_ -> undefined
	end,
	
	case {T, Rel} of
		{undefined, _} -> {error, invalid_vlu};
		{_, undefined} -> {error, invalid_rln};
		{_, _} -> {ok, {Atm, Rel, T}}
	end.

form_recipe_cond_atm_crv(Atm, P) ->
	T = case proplists:get_value(<<"vlu">>, P) of
		S when is_binary(S) ->
			binary_to_atom(S, utf8);
		_ ->
			undefined
	end,
	Rel = case proplists:get_value(<<"rln">>, P) of
		<<"=">> -> '=';
		<<"!=">> -> '!=';
		_ -> undefined
	end,
	
	case {T, Rel} of
		{undefined, _} -> {error, invalid_vlu};
		{_, undefined} -> {error, invalid_rln};
		{_, _} -> {ok, {Atm, Rel, T}}
	end.

read_recipe_action(<<"add_skills">>, P) ->
	case proplists:get_value(<<"actionValue">>, P) of
		{array, SkillBins} ->
			Skills = [binary_to_atom(Bin, utf8) || Bin <- SkillBins],
			{ok, {add_skills, Skills}};
		_ ->
			{error, invalid_vlu}
	end;
read_recipe_action(<<"remove_skills">>, P) ->
	case proplists:get_value(<<"actionValue">>, P) of
		{array, SkillBins} ->
			Skills = [binary_to_atom(Bin, utf8) || Bin <- SkillBins],
			{ok, {remove_skills, Skills}};
		_ ->
			{error, invalid_vlu}
	end;
read_recipe_action(<<"set_priority">>, P) ->
	case proplists:get_value(<<"actionValue">>, P) of
		N when is_number(N) ->
			{ok, {set_priority, erlang:trunc(N)}};
		_ ->
			{error, invalid_vlu}
	end;
read_recipe_action(<<"prioritize">>, _) ->
	{ok, {prioritize, []}};
read_recipe_action(<<"deprioritize">>, _) ->
	{ok, {deprioritize, []}};
read_recipe_action(<<"voicemail">>, _) ->
	{ok, {voicemail, []}};
read_recipe_action(<<"announce">>, P) ->
	case proplists:get_value(<<"actionValue">>, P) of
		Bin when is_binary(Bin) ->
			{ok, {announce, binary_to_list(Bin)}};
		_ ->
			{error, invalid_vlu}
	end.

build_skill(P) ->
	R = build_skill(P, #skill_rec{name=undefined, description=""}),
	case {R#skill_rec.atom, R#skill_rec.name} of
		{undefined, _} -> {error, noatom};
		{_, undefined} -> {error, noname};
		_ -> {ok, R}
	end.

build_skill([], Acc) ->
	Acc;
build_skill([{<<"atom">>, Atm}|T], Acc) ->
	build_skill(T, Acc#skill_rec{atom = binary_to_atom(Atm, utf8)});
build_skill([{<<"name">>, Name}|T], Acc) ->
	build_skill(T, Acc#skill_rec{name = binary_to_list(Name)});
build_skill([{<<"dscr">>, Desc}|T], Acc) ->
	build_skill(T, Acc#skill_rec{description = binary_to_list(Desc)});
build_skill([{<<"grpnm">>, Group}|T], Acc) ->
	build_skill(T, Acc#skill_rec{group = binary_to_list(Group)});
build_skill([_|T], Acc) ->
	build_skill(T, Acc).

build_client(P) ->
	R = build_client(P, #client{label=undefined}),
	case {R#client.id, R#client.label} of
		{undefined, _} -> {error, noid};
		{_, undefined} -> {error, nolabel};
		_ -> {ok, R}
	end.

build_client([], Acc) ->
	Acc;
build_client([{<<"ident">>, Id}|T], Acc) ->
	build_client(T, Acc#client{id = binary_to_list(Id)});
build_client([{<<"name">>, Label}|T], Acc) ->
	build_client(T, Acc#client{label = binary_to_list(Label)});
build_client([{<<"autoendwrp">>, Secs}|T], Acc)
		when is_integer(Secs) andalso Secs > 0 ->
	Opts = Acc#client.options,
	build_client(T, Acc#client{options = [{autoend_wrapup, Secs}|Opts]});
build_client([_|T], Acc) ->
	build_client(T, Acc).

-ifdef(TEST).
%%--------------------------------------------------------------------
%%% Test functions
%%--------------------------------------------------------------------

build_agent_test_() ->
	Build = fun(L) -> spx_util:build_agent([{<<"_id">>, <<"fooid">>}|L]) end,
	[
		%% id
		?_assertEqual({error, noid}, spx_util:build_agent([])),
		?_assertMatch({ok, #agent_auth{id="fooid"}},
			Build([])),
		
		%% login
		?_assertMatch({ok, #agent_auth{login="foo"}},
			Build([{<<"name">>, <<"foo">>}])),
		
		%% skills
		?_assertMatch({ok, #agent_auth{skills=[]}}, Build([])),
		?_assertMatch({ok, #agent_auth{
				skills=['_agent', english,
					{'_queue', "cyber"}, {'_queue', "mega"},
					{'_brand', "dalek"}, {'_brand', "master"}]}},
			Build([
				{<<"skl">>, {array,[<<"_agent">>, <<"english">>]}},
				{<<"qs">>, {array,[<<"cyber">>, <<"mega">>]}},
				{<<"clns">>,{array,[<<"dalek">>, <<"master">>]}}
			])),
		
		%% security level
		?_assertMatch({ok, #agent_auth{securitylevel=agent}}, Build([])),
		?_assertMatch({ok, #agent_auth{securitylevel=admin}}, 
			Build([{<<"scrty">>,<<"ADMIN">>}])),
		?_assertMatch({ok, #agent_auth{securitylevel=supervisor}}, 
			Build([{<<"scrty">>,<<"SUPERVISOR">>}])),

		%% profile
		?_assertMatch({ok, #agent_auth{profile="Default"}}, Build([])),
		?_assertMatch({ok, #agent_auth{profile="alien"}},
			Build([{<<"aggrp">>, <<"alien">>}])),

		%% firstname
		?_assertMatch({ok, #agent_auth{firstname="Sam"}},
			Build([{<<"fnm">>, <<"Sam">>}])),

		%% lastname
		?_assertMatch({ok, #agent_auth{lastname="Sung"}},
			Build([{<<"lnm">>, <<"Sung">>}])),

		%% endpoints
		?_assertMatch({ok, #agent_auth{endpoints=[]}},
			Build([])),
		?_assertMatch({ok, #agent_auth{endpoints=[
				{freeswitch_media, [{type, pstn},
					{data, "201@sipfoundry.org"}]},
				{freeswitch_voicemail, [{type, pstn},
					{data, "201@sipfoundry.org"}]}
				]}},
			Build([{<<"cnt">>, <<"201@sipfoundry.org">>}])),		

		% unknown
		?_assertMatch({ok, _}, Build([{<<"unknownprop">>, <<"blabber">>}]))
		  
	].

build_profile_test_() ->
	Build = fun(L) -> spx_util:build_profile([{<<"_id">>, <<"bazid">>},
		{<<"name">>, <<"baz">>}|L]) end,
	[
		%% id/name
		?_assertEqual({error, noid}, spx_util:build_profile([])),
		?_assertEqual({error, noname}, spx_util:build_profile([{<<"_id">>, <<"b">>}])),
		?_assertMatch({ok, #agent_profile{name="baz"}}, Build([])),
	
		%% skills		
		?_assertMatch({ok, #agent_profile{skills=[]}}, Build([])),
		?_assertMatch({ok, #agent_profile{
				skills=['_agent', english,
					{'_queue', "cyber"}, {'_queue', "mega"},
					{'_brand', "dalek"}, {'_brand', "master"}]}},
			Build([
				{<<"skl">>, {array,[<<"_agent">>, <<"english">>]}},
				{<<"qs">>, {array,[<<"cyber">>, <<"mega">>]}},
				{<<"clns">>,{array,[<<"dalek">>, <<"master">>]}}
			])),

		% unknown
		?_assertMatch({ok, _}, Build([{<<"unknownprop">>, <<"blabber">>}]))
	].

build_release_opt_test_() ->
	Build = fun(P) -> spx_util:build_release_opt([{<<"_id">>, <<"relid1">>}, {<<"lbl">>, <<"in a meeting">>}|P]) end,

	[?_assertEqual({error, noid}, spx_util:build_release_opt([])),
	?_assertEqual({error, invalidid}, spx_util:build_release_opt([{<<"_id">>, <<"nonumid">>}])),
	?_assertEqual({error, nolabel}, spx_util:build_release_opt([{<<"_id">>, <<"relid1">>}])),

	?_assertMatch({ok, #release_opt{id=1, label="in a meeting"}},
		Build([])),

	?_assertMatch({ok, #release_opt{bias = 0}}, Build([])),
	?_assertMatch({ok, #release_opt{bias = 1}}, Build([{<<"bias">>, 1}])),

	?_assertMatch({ok, #release_opt{}}, Build([{<<"unknown">>, 0}]))
	].

build_queue_test_() ->
	Build = fun(L) -> spx_util:build_queue([{<<"name">>, <<"q">>}|L]) end,
	[
		?_assertEqual({error, noname}, spx_util:build_queue([])),
		?_assertMatch({ok, #call_queue{name="q"}}, Build([])),

		?_assertMatch({ok, #call_queue{weight=1}}, Build([])),
		?_assertMatch({ok, #call_queue{weight=10}}, Build([{<<"wht">>, 10}])),

		?_assertMatch({ok, #call_queue{group="Default"}}, Build([])), %% TODO not accept this?
		?_assertMatch({ok, #call_queue{group="mygroup"}}, Build([{<<"qgrp">>, <<"mygroup">>}])),

		%% skills
		?_assertMatch({ok, #call_queue{skills=[]}}, Build([])),
		?_assertMatch({ok, #call_queue{
			skills=['_agent', english,
					{'_queue', "cyber"}, {'_queue', "mega"},
					{'_brand', "dalek"}, {'_brand', "master"}]}},
			Build([
				{<<"skl">>, {array,[<<"_agent">>, <<"english">>]}},
				{<<"qs">>, {array,[<<"cyber">>, <<"mega">>]}},
				{<<"clns">>,{array,[<<"dalek">>, <<"master">>]}}
			])),

		%% TODO add recipe

		%% unknown
		?_assertMatch({ok, #call_queue{}}, Build([{<<"unknownprop">>, <<"someval">>}]))

	].

build_queue_group_test_() ->
	Build = fun(L) -> spx_util:build_queue_group([{<<"name">>, <<"qg">>}|L]) end,
	[
		%% name
		?_assertEqual({error, noname}, spx_util:build_queue_group([])),
		?_assertMatch({ok, #queue_group{name="qg"}}, Build([])),

		%% skills
		?_assertMatch({ok, #queue_group{skills=[]}}, Build([])),
		?_assertMatch({ok, #queue_group{
			skills=['_agent', english,
					{'_queue', "cyber"}, {'_queue', "mega"},
					{'_brand', "dalek"}, {'_brand', "master"}]}},
			Build([
				{<<"skl">>, {array,[<<"_agent">>, <<"english">>]}},
				{<<"qs">>, {array,[<<"cyber">>, <<"mega">>]}},
				{<<"clns">>,{array,[<<"dalek">>, <<"master">>]}}
			])),		

		%% recipe
		?_assertMatch({ok, #queue_group{recipe=[]}}, Build([])),

		%% protected - always false
		?_assertMatch({ok, #queue_group{protected=false}}, Build([])),

		%% invalid property ignored
		?_assertMatch({ok, #queue_group{}}, Build([{<<"unknownprop">>, <<"someval">>}]))
	].

% -type(recipe_step() ::
% 	{[recipe_condition(), ...], [recipe_operation(), ...], recipe_runs(), recipe_comment()}
% ).
build_recipe_test_() ->
	CondsOnlyExp = fun(Conds) -> [{Conds, [], run_once, <<>>}] end,
	BuildRecipeWithSingleCond = fun(Cndt, Cond) -> 
		build_recipe([[{<<"cndt">>, {array, [[{<<"cndt">>, Cndt}|Cond]]}}]])
	end,

	BuildRecipeWithAction = fun(Action, ActionValue) ->
		build_recipe([[{<<"actn">>, [{<<"action">>, Action}, {<<"actionValue">>,
				ActionValue}]}]])
	end,
	ActionOnlyExp = fun(Action) ->
		[{[], [Action], run_once, <<>>}]
	end,

	EmptyStep = [{[], [], run_once, <<>>}],

	[
		?_assertEqual([], build_recipe([])),

		{"empty step", ?_assertEqual([{[], [], run_once, <<>>}], build_recipe([[]]))},
		{"frequency", ?_assertEqual([{[], [], run_many, <<>>}], build_recipe([[{<<"frq">>, <<"run_many">>}]]))},
		{"name", ?_assertEqual([{[], [], run_once, <<"newstep">>}], build_recipe([[{<<"stpnm">>, <<"newstep">>}]]))},
		{"unknow prop", ?_assertEqual([{[], [], run_once, <<>>}], build_recipe([[{<<"family">>, <<"reptile">>}]]))},

		{"ticks",
		[?_assertEqual(CondsOnlyExp([{ticks, 10}]),
			BuildRecipeWithSingleCond(<<"ticks">>, [{<<"vlu">>, 10.0}])),
		?_assertEqual(CondsOnlyExp([]),
			BuildRecipeWithSingleCond(<<"ticks">>, []))]},

		num_crv_cond_test_desc(available_agents),
		num_crv_cond_test_desc(eligible_agents),
		num_crv_cond_test_desc(queue_position),
		num_crv_cond_test_desc(calls_queued),
		num_crv_cond_test_desc(client_calls_queued),
		num_crv_cond_test_desc(hour),
		num_crv_cond_test_desc(weekday),

		str_crv_cond_test_desc(client),
		str_crv_cond_test_desc(caller_name),
		str_crv_cond_test_desc(caller_id),
		
		atm_crv_cond_test_desc(type),
		%% TODO custom conditions

		?_assertEqual(CondsOnlyExp([{ticks, 10}, {available_agents, '>', 5}]),
			build_recipe([[{<<"cndt">>,
				{array, [[{<<"cndt">>, <<"ticks">>}, {<<"vlu">>, 10}],
				         [{<<"cndt">>, <<"available_agents">>},
				          {<<"rln">>, <<">">>},
				          {<<"vlu">>, 5}]]}}]])),

	
		?_assertEqual(ActionOnlyExp({add_skills, [english, german]}),
			BuildRecipeWithAction(<<"add_skills">>, 
				{array, [<<"english">>, <<"german">>]})),
		?_assertEqual(EmptyStep, BuildRecipeWithAction(<<"add_skills">>, 5.0)),

		?_assertEqual(ActionOnlyExp({remove_skills, [english, german]}),
			BuildRecipeWithAction(<<"remove_skills">>, 
				{array, [<<"english">>, <<"german">>]})),
		?_assertEqual(EmptyStep, BuildRecipeWithAction(<<"remove_skills">>, 5.0)),

		?_assertEqual(ActionOnlyExp({set_priority, 5}),
			BuildRecipeWithAction(<<"set_priority">>, 5.0)),
		?_assertEqual(EmptyStep,
			BuildRecipeWithAction(<<"set_priority">>, "abc")),

		?_assertEqual(ActionOnlyExp({prioritize, []}),
			BuildRecipeWithAction(<<"prioritize">>, [])),
		?_assertEqual(ActionOnlyExp({deprioritize, []}),
			BuildRecipeWithAction(<<"deprioritize">>, [])),
		?_assertEqual(ActionOnlyExp({voicemail, []}),
			BuildRecipeWithAction(<<"voicemail">>, [])),

		?_assertEqual(ActionOnlyExp({announce, "helloworld"}),
			BuildRecipeWithAction(<<"announce">>, <<"helloworld">>)),
		?_assertEqual(EmptyStep,
			BuildRecipeWithAction(<<"announce">>, 123))

		%% TODO custom operations
	].

build_skill_test_() ->
	Build = fun(P) -> spx_util:build_skill([{<<"atom">>, <<"skillz">>}, {<<"name">>, <<"fave">>}|P]) end,
	[
		?_assertEqual({error, noatom}, spx_util:build_skill([])),

		?_assertMatch({ok, #skill_rec{atom=skillz}}, Build([])),

		?_assertMatch({error, noname}, spx_util:build_skill([{<<"atom">>, <<"skillz">>}])),
		?_assertMatch({ok, #skill_rec{name="fave"}}, Build([])),

		?_assertMatch({ok, #skill_rec{description=""}}, Build([])),
		?_assertMatch({ok, #skill_rec{description="favorite"}}, Build([{<<"dscr">>, <<"favorite">>}])),

		?_assertMatch({ok, #skill_rec{group="Misc"}}, Build([])),
		?_assertMatch({ok, #skill_rec{group="Magic"}}, Build([{<<"grpnm">>, <<"Magic">>}])),

		?_assertMatch({ok, #skill_rec{}}, Build([{<<"unknown">>, <<"prop">>}]))
	].

build_client_test_() ->
	Build = fun(P) -> spx_util:build_client([{<<"ident">>, <<"client1">>}, {<<"name">>, <<"MyClient">>}|P]) end,
	[
		?_assertEqual({error, noid}, spx_util:build_client([])),
		
		?_assertEqual({error, nolabel}, spx_util:build_client([{<<"ident">>, <<"client1">>}])),
		?_assertMatch({ok, #client{label="MyClient"}}, Build([])),

		?_assertMatch({ok, #client{options=[]}}, Build([])),
		?_assertMatch({ok, #client{options=[]}}, Build([{<<"autoendwrp">>, null}])),
		?_assertMatch({ok, #client{options=[{autoend_wrapup, 10}]}}, Build([{<<"autoendwrp">>, 10}])),

		?_assertMatch({ok, #client{}}, Build([{<<"unknown">>, <<"prop">>}]))
	].


num_crv_cond_test_desc(Atm) ->
	Bin = atom_to_binary(Atm, utf8),
	Str = atom_to_list(Atm),

	CondsOnlyExp = fun(Conds) -> [{Conds, [], run_once, <<>>}] end,
	BuildRecipeWithSingleCond = fun(Cndt, Cond) -> 
		build_recipe([[{<<"cndt">>, {array, [[{<<"cndt">>, Cndt}|Cond]]}}]])
	end,

	BuildRecipeWithSingleCRV = fun(Cndt, Rln, Vlu) ->
		BuildRecipeWithSingleCond(Cndt, [{<<"rln">>, Rln}, {<<"vlu">>, Vlu}])
	end,

	{Str, [?_assertEqual(CondsOnlyExp([{Atm, '>', 10}]),
		BuildRecipeWithSingleCRV(Bin, <<">">>, 10.0)),
	?_assertEqual(CondsOnlyExp([{Atm, '<', 10}]),
		BuildRecipeWithSingleCRV(Bin, <<"<">>, 10.0)),
	?_assertEqual(CondsOnlyExp([{Atm, '=', 10}]),
		BuildRecipeWithSingleCRV(Bin, <<"=">>, 10.0)),
	?_assertEqual(CondsOnlyExp([]),
		BuildRecipeWithSingleCRV(Bin, <<"$$">>, 10.0)),
	?_assertEqual(CondsOnlyExp([]),
		BuildRecipeWithSingleCRV(Bin, <<"=">>, -5.0))]}.

str_crv_cond_test_desc(Atm) ->
	Bin = atom_to_binary(Atm, utf8),
	Str = atom_to_list(Atm),

	CondsOnlyExp = fun(Conds) -> [{Conds, [], run_once, <<>>}] end,
	BuildRecipeWithSingleCond = fun(Cndt, Cond) ->
		build_recipe([[{<<"cndt">>, {array, [[{<<"cndt">>, Cndt}|Cond]]}}]])
	end,

	BuildRecipeWithSingleCRV = fun(Cndt, Rln, Vlu) ->
		BuildRecipeWithSingleCond(Cndt, [{<<"rln">>, Rln}, {<<"vlu">>, Vlu}])
	end,

	{Str, [?_assertEqual(CondsOnlyExp([{Atm, '=', "a"}]),
		BuildRecipeWithSingleCRV(Bin, <<"=">>, <<"a">>)),
	?_assertEqual(CondsOnlyExp([{Atm, '!=', "a"}]),
		BuildRecipeWithSingleCRV(Bin, <<"!=">>, <<"a">>)),
	?_assertEqual(CondsOnlyExp([]),
		BuildRecipeWithSingleCRV(Bin, <<">">>, <<"b">>)),
	?_assertEqual(CondsOnlyExp([]),
		BuildRecipeWithSingleCRV(Bin, <<"=">>, -5.0))]}.

atm_crv_cond_test_desc(Atm) ->
	Bin = atom_to_binary(Atm, utf8),
	Str = atom_to_list(Atm),

	CondsOnlyExp = fun(Conds) -> [{Conds, [], run_once, <<>>}] end,
	BuildRecipeWithSingleCond = fun(Cndt, Cond) ->
		build_recipe([[{<<"cndt">>, {array, [[{<<"cndt">>, Cndt}|Cond]]}}]])
	end,

	BuildRecipeWithSingleCRV = fun(Cndt, Rln, Vlu) ->
		BuildRecipeWithSingleCond(Cndt, [{<<"rln">>, Rln}, {<<"vlu">>, Vlu}])
	end,

	{Str, [?_assertEqual(CondsOnlyExp([{Atm, '=', voice}]),
		BuildRecipeWithSingleCRV(Bin, <<"=">>, <<"voice">>)),
	?_assertEqual(CondsOnlyExp([{Atm, '!=', voice}]),
		BuildRecipeWithSingleCRV(Bin, <<"!=">>, <<"voice">>)),
	?_assertEqual(CondsOnlyExp([]),
		BuildRecipeWithSingleCRV(Bin, <<">">>, <<"voice">>)),
	?_assertEqual(CondsOnlyExp([]),
		BuildRecipeWithSingleCRV(Bin, <<"=">>, -5.0))]}.

-endif.
