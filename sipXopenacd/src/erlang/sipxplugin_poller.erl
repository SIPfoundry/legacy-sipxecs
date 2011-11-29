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

-module(sipxplugin_poller).
-author("eZuce").

-behavior(gen_server).

%% API
-export([start/0, stop/0,
	get_last_poll_time/0]).

%% gen_server callbacks
-export([init/1, handle_call/3, handle_cast/2, handle_info/2,
         terminate/2, code_change/3]).

-record(state, {timer, last_poll_time}).
-define(SERVER, ?MODULE).

-include("log.hrl").
-include("cpx.hrl").
-include("queue.hrl").
-include("call.hrl").
-include("agent.hrl").

start() ->
	gen_server:start_link({local, ?SERVER}, ?SERVER, [], []).

stop() ->
	gen_server:call(?SERVER, stop).

get_last_poll_time() ->
	gen_server:call(?SERVER, get_last_poll_time).
%%====================================================================
%% gen_server callbacks
%%====================================================================

%%--------------------------------------------------------------------
%% Function: init(Args) -> {ok, State} |
%%                         {ok, State, Timeout} |
%%                         ignore               |
%%                         {stop, Reason}
%% Description: Initiates the server
%%--------------------------------------------------------------------
init([]) ->
	mongodb:singleServer(def),
	mongodb:connect(def),
	{ok, Timer} = timer:send_interval(10000, tick),
	State = #state{timer = Timer},

	{ok, State}.
%--------------------------------------------------------------------
%% Function: %% handle_call(Request, From, State) -> {reply, Reply, State} |
%%                                      {reply, Reply, State, Timeout} |
%%                                      {noreply, State} |
%%                                      {noreply, State, Timeout} |
%%                                      {stop, Reason, Reply, State} |
%%                                      {stop, Reason, State}
%% Description: Handling call messages
%%--------------------------------------------------------------------
handle_call(get_last_poll_time, _From, State) ->
	{reply, State#state.last_poll_time, State};
handle_call(stop, _From, State) ->
	{stop, normal, ok, State};
handle_call(_Request, _From, State) ->
	Reply = ok,
	{reply, Reply, State}.

%%--------------------------------------------------------------------
%% Function: handle_cast(Msg, State) -> {noreply, State} |
%%                                      {noreply, State, Timeout} |
%%                                      {stop, Reason, State}
%% Description: Handling cast messages
%%--------------------------------------------------------------------
handle_cast(_Msg, State) ->
  {noreply, State}.

%%--------------------------------------------------------------------
%% Function: handle_info(Info, State) -> {noreply, State} |
%%                                       {noreply, State, Timeout} |
%%                                       {stop, Reason, State}
%% Description: Handling all non call/cast messages
%%--------------------------------------------------------------------
handle_info(tick, State) ->
	PollTime = erlang:now(),
	get_new_config(),
	{noreply, State#state{last_poll_time = PollTime}};

handle_info(_Info, State) ->
  {noreply, State}.

%%--------------------------------------------------------------------
%% Function: terminate(Reason, State) -> void()
%% Description: This function is called by a gen_server when it is about to
%% terminate. It should be the opposite of Module:init/1 and do any necessary
%% cleaning up. When it returns, the gen_server terminates with Reason.
%% The return value is ignored.
%%--------------------------------------------------------------------
terminate(_Reason, _State) ->
  ok.

%%--------------------------------------------------------------------
%% Func: code_change(OldVsn, State, Extra) -> {ok, NewState}
%% Description: Convert process state when code is changed
%%--------------------------------------------------------------------
code_change(_OldVsn, State, _Extra) ->
  {ok, State}.

%%--------------------------------------------------------------------
%%% Internal functions
%%--------------------------------------------------------------------

get_new_config() ->
	%connect to openacd db and count objects in commands collection
	Mong = mongoapi:new(def,<<"openacd">>),
	
	case Mong:find("commands", [], undefined, 0, 0) of
		{ok, []} ->
			?DEBUG("No Command to execute", []);
		{ok, Commands} ->
			lists:foreach(fun(Cmd) ->
				get_command_values(Cmd, Mong)
			end, Commands)
	end.
	
get_command_values(Data, Mong) ->
	case Data of
		[] ->
			?DEBUG("No Data", []);
		_ ->
			% command format { "_id" : ObjectId("4ce62e892957ca4fc97387a1"), "command" : "ADD", "count" : 2, "objects" : []}
			?DEBUG("Processing Mongo DB Command: ~p", [Data]),

			Id = proplists:get_value(<<"_id">>, Data),
			CommandBin = proplists:get_value(<<"command">>, Data),
			{array, Objects} = proplists:get_value(<<"objects">>, Data),

			Command = binary_to_list(CommandBin),

			lists:foreach(fun(Object) ->
				% objects to process starts with type e.g. "type" : "agent", "name" : "bond", "pin" : "1234"

				Type = proplists:get_value(<<"type">>, Object),

				case Type of
					<<"agent">> ->
						process_agent(Object, Command);
					<<"profile">> ->
						process_profile(Object, Command);
					<<"skill">> ->
						process_skill(Object, Command);
					<<"client">> ->
						process_client(Object, Command);
					<<"queueGroup">> ->
						process_queue_group(Object, Command);
					<<"queue">> ->
						process_queue(Object, Command);
					<<"freeswitch_media_manager">> ->
						process_fs_media_manager(Object, Command);
					<<"agent_configuration">> ->
						process_agent_configuration(Object, Command);
					<<"log_configuration">> ->
						process_log_configuration(Object, Command);
					<<"vm_priority_diff">> ->
						process_vm_priority_diff(Object, Command);
					_ ->
						?WARNING("Unrecognized type", [])
				end
			end, Objects),

			Mong:runCmd([{"findandmodify", "commands"},{"query", [{"_id",Id}]},{"remove",1}])
	end.

process_agent(Agent, "ADD") ->
	agent_auth:add_agent(
		get_str(<<"name">>, Agent),
		get_str(<<"firstName">>, Agent),
		get_str(<<"lastName">>, Agent),
		get_str(<<"pin">>, Agent),
		get_all_skills(Agent),
		get_agent_security(Agent),
		get_str(<<"group">>, Agent));

process_agent(Agent, "DELETE") ->
	agent_auth:destroy(
		get_str(<<"name">>, Agent));

process_agent(Agent, "UPDATE") ->
	OldName = get_str(<<"oldName">>, Agent),
	case agent_auth:get_agent(OldName) of
		{atomic, [Old]} ->
			%% TODO must be an atomic operation
			Id = Old#agent_auth.id,			
			agent_auth:set_agent(Id,
				get_str(<<"name">>, Agent),
				get_str(<<"pin">>, Agent),
				get_all_skills(Agent),
				get_agent_security(Agent),
				get_str(<<"group">>, Agent),
				get_str(<<"firstName">>, Agent),
				get_str(<<"lastName">>, Agent));
		{atomic, []} ->
			?WARNING("Failed to update non-existing agent: ~s",
				[OldName])
	end;
process_agent(_, Command) ->
	?WARNING("Unrecognized command: ~s", [Command]),
	{error, unkown_command}.

process_profile(Profile, "ADD") ->
	agent_auth:new_profile(get_str(<<"name">>, Profile),
		get_all_skills(Profile));

process_profile(Profile, "DELETE") ->
	agent_auth:destroy_profile(get_str(<<"name">>, Profile));

process_profile(Profile, "UPDATE") ->
	agent_auth:set_profile(
		get_str(<<"oldName">>, Profile),
		get_str(<<"name">>, Profile),
		get_all_skills(Profile));

process_profile(_, Command) ->
	?WARNING("Unrecognized command: ~s", [Command]),
	{error, unkown_command}.

process_skill(Skill, Command) ->
	{_, Name} = lists:nth(2, Skill),
	{_, Atom} = lists:nth(3, Skill),
	{_, Group} = lists:nth(4, Skill),
	{_, Description} = lists:nth(5, Skill),
	if Description =:= null ->
		Descr = "";
	true -> Descr = erlang:binary_to_list(Description)
	end,
	if Command =:= "ADD" ->
		call_queue_config:new_skill(list_to_atom(erlang:binary_to_list(Atom)), erlang:binary_to_list(Name), Descr, erlang:binary_to_list(Group));
	Command =:= "DELETE" ->
		call_queue_config:destroy_skill(erlang:binary_to_list(Name));
	Command =:= "UPDATE" ->
		call_queue_config:set_skill(list_to_atom(erlang:binary_to_list(Atom)), erlang:binary_to_list(Name), Descr, erlang:binary_to_list(Group));
	true -> ?WARNING("Unrecognized command", [])
	end.

process_client(Client, Command) ->
	{_, Name} = lists:nth(2, Client),
	{_, Identity} = lists:nth(3, Client),
	if Command =:= "ADD" ->
		call_queue_config:new_client(erlang:binary_to_list(Name), erlang:binary_to_list(Identity), []);
	Command =:= "DELETE" ->
		call_queue_config:destroy_client(erlang:binary_to_list(Identity));
	Command =:= "UPDATE" ->
		call_queue_config:set_client(erlang:binary_to_list(Identity), erlang:binary_to_list(Name), []);
	true -> ?WARNING("Unrecognized command", [])
	end.

process_queue_group(QueueGroup, Command) ->
	{_, Name} = lists:nth(2, QueueGroup),
	{_, Skills} = lists:nth(3, QueueGroup),
	{_, Profiles} = lists:nth(4, QueueGroup),
	SkillsList = lists:flatmap(fun(X)->[list_to_atom(X)] end, string:tokens((erlang:binary_to_list(Skills)), ", ")),
	ProfilesList = lists:flatmap(fun(X)->[{'_profile',X}] end, string:tokens((erlang:binary_to_list(Profiles)), ", ")),
	AllSkills = lists:merge(SkillsList, ProfilesList),
	if Command =:= "ADD" ->
		NewQgroup = #queue_group{name = erlang:binary_to_list(Name), sort = 10, recipe = [], skills = AllSkills},
		call_queue_config:new_queue_group(NewQgroup);
	Command =:= "DELETE" ->
		call_queue_config:destroy_queue_group(erlang:binary_to_list(Name));
	Command =:= "UPDATE" ->
		{_, Oldname} = lists:nth(5, QueueGroup),
		{_, [{_, _, OldRecipe, _, _, _, _}]} = call_queue_config:get_queue_group(erlang:binary_to_list(Oldname)),
		Qgroup = #queue_group{name = erlang:binary_to_list(Name), sort = 10, recipe = OldRecipe, skills = AllSkills},
		call_queue_config:set_queue_group(erlang:binary_to_list(Oldname), Qgroup);
	true -> ?WARNING("Unrecognized command", [])
	end.

process_queue(Queue, Command) ->
	{_, Name} = lists:nth(2, Queue),
	{_, QueueGroup} = lists:nth(3, Queue),
	{_, Skills} = lists:nth(4, Queue),
	{_, Profiles} = lists:nth(5, Queue),
	SkillsList = lists:flatmap(fun(X)->[list_to_atom(X)] end, string:tokens((erlang:binary_to_list(Skills)), ", ")),
	ProfilesList = lists:flatmap(fun(X)->[{'_profile',X}] end, string:tokens((erlang:binary_to_list(Profiles)), ", ")),
	AllSkills = lists:merge(SkillsList, ProfilesList),
	{_, Weight} = lists:nth(6, Queue),
	if Command =:= "ADD" ->
		{_, {_, RecipeSteps}} = lists:nth(8, Queue),
		if RecipeSteps =:= [] -> RecipeToSave = [];
			true ->
				RecipeToSave = lists:flatmap(fun(X) -> [extract_recipe_step(X)] end, RecipeSteps)
		end,
		call_queue_config:new_queue(erlang:binary_to_list(Name), binary_to_number(Weight), AllSkills, RecipeToSave, erlang:binary_to_list(QueueGroup)),
		queue_manager:load_queue(erlang:binary_to_list(Name));
	Command =:= "DELETE" ->
		call_queue_config:destroy_queue(erlang:binary_to_list(Name));
	Command =:= "UPDATE" ->
		{_, Oldname} = lists:nth(7, Queue),
		{_, {_, RecipeSteps}} = lists:nth(8, Queue),
		if RecipeSteps =:= [] -> RecipeToSave = [];
			true ->
				RecipeToSave = lists:flatmap(fun(X) -> [extract_recipe_step(X)] end, RecipeSteps)
		end,
		call_queue_config:set_queue(erlang:binary_to_list(Oldname), erlang:binary_to_list(Name), binary_to_number(Weight), AllSkills, RecipeToSave, erlang:binary_to_list(QueueGroup)),
		queue_manager:load_queue(erlang:binary_to_list(Name));
	true -> ?WARNING("Unrecognized command", [])
	end.

process_fs_media_manager(Config, _Command) ->
        {_, Enabled} = lists:nth(2, Config),
        {_, CNode} = lists:nth(3, Config),
        {_, DialString} = lists:nth(4, Config),
        if Enabled =:= <<"true">> ->
		Conf = #cpx_conf{id = freeswitch_media_manager, module_name = freeswitch_media_manager, start_function = start_link, start_args = [list_to_atom(erlang:binary_to_list(CNode)), [{h323,[]}, {iax2,[]}, {sip,[]}, {dialstring,erlang:binary_to_list(DialString)}]], supervisor = mediamanager_sup},
                cpx_supervisor:update_conf(freeswitch_media_manager, Conf);
        Enabled =:= <<"false">> ->
                cpx_supervisor:destroy(freeswitch_media_manager);
        true -> ?WARNING("Unrecognized command", [])
        end.

process_agent_configuration(Config, _Command) ->
        {_, ListenerEnabled} = lists:nth(2, Config),
        if ListenerEnabled =:= <<"true">> ->
		Conf = #cpx_conf{id = agent_dialplan_listener, module_name = agent_dialplan_listener, start_function = start_link, start_args = [], supervisor = agent_connection_sup},
                cpx_supervisor:update_conf(agent_dialplan_listener, Conf);
        ListenerEnabled =:= <<"false">> ->
                cpx_supervisor:destroy(agent_dialplan_listener);
        true -> ?WARNING("Unrecognized command", [])
        end.

process_log_configuration(Config, _Command) ->
        {_, LogLevel} = lists:nth(2, Config),
        {_, LogDir} = lists:nth(3, Config),
	LogLevelAtom = list_to_atom(erlang:binary_to_list(LogLevel)),
	?WARNING("SET NEW LOG LEVEL:~p", [list_to_atom(erlang:binary_to_list(LogLevel))]),
	cpxlog:set_loglevel(lists:append(erlang:binary_to_list(LogDir), "full.log"), LogLevelAtom),
	cpxlog:set_loglevel(lists:append(erlang:binary_to_list(LogDir), "console.log"), LogLevelAtom).

extract_condition(MongoCondition) ->
	[{_, Condition}, {_, Relation}, {_, ConditionValue}] = MongoCondition,
	ConditionAtom = list_to_existing_atom(binary_to_list(Condition)),
	RelationAtom = list_to_existing_atom(binary_to_list(Relation)),

	case ConditionAtom of
		client ->
			Client = binary_to_list(ConditionValue),
			{client, RelationAtom, Client};
		type ->
			%% TODO may cause a memory problem. must handle non-existing atoms
			Type = list_to_atom(binary_to_list(ConditionValue)),
			{type, RelationAtom, Type};
		ticks ->
			Ticks = binary_to_number(ConditionValue),
			{ticks, Ticks};
		_ ->
			%% would probably be best to handle each type
			Num = binary_to_number(ConditionValue),
			{ConditionAtom, Num}
	end.

process_vm_priority_diff(Object, _Command) ->
	case proplists:get_value(<<"diff">>, Object) of
		Diff when is_number(Diff) ->
			DiffI = trunc(Diff),
			?DEBUG("Setting vm priority diff to ~b", [DiffI]),
			cpx_supervisor:set_value(vm_priority_diff, DiffI);
		_ ->
			ok
	end.

extract_recipe_step(RecipeStep) ->
	[{_, [{_, RecipeAction}, {_, RecipeActionValue}]}, {_, {_, RecipeConditions}}, {_, RecipeFrequency}, {_, RecipeName}] = RecipeStep,
	RecipeActionAtom = list_to_atom(erlang:binary_to_list(RecipeAction)),
	if RecipeActionAtom =:= announce ->
		RecipeActionValueAtom = erlang:binary_to_list(RecipeActionValue);
	RecipeActionAtom =:= set_priority ->
		RecipeActionValueAtom = binary_to_number(RecipeActionValue);
	(RecipeActionAtom =:= add_skills) or (RecipeActionAtom =:= remove_skills) ->
		RecipeActionValueAtom = lists:flatmap(fun(X)->[list_to_atom(X)] end, string:tokens((erlang:binary_to_list(RecipeActionValue)), ", "));
		true -> RecipeActionValueAtom = []
		end,
	ActionToSave = {RecipeActionAtom, RecipeActionValueAtom},
	ConditionList = lists:flatmap(fun(X) -> [extract_condition(X)] end, RecipeConditions),
	{ConditionList,[ActionToSave],list_to_atom(erlang:binary_to_list(RecipeFrequency)),RecipeName}.


%% Utils


get_all_skills(Props) ->
	Skills = get_atom_list(<<"skillsAtoms">>, Props),
	Queues = [{'_queue', X} || 
		X <- get_atom_list(<<"queuesName">>, Props)],
	Clients = [{'_brand', X} ||
		X <- get_atom_list(<<"clientsName">>, Props)],
    Skills ++ Queues ++ Clients.

get_agent_security(Agent) ->
	SecurityBin = get_bin(<<"security">>, Agent),
    case SecurityBin of
    	<<"SUPERVISOR">> ->
    		supervisor;
    	<<"ADMIN">> ->
    		admin;
    	_ ->
    		agent
    end.

get_str(Key, L) ->
	case proplists:get_value(Key, L) of
		undefined ->
			"";
		Bin ->
			%% TODO use proper encoding
			binary_to_list(Bin)
	end.

get_bin(Key, L) ->
	proplists:get_value(Key, L, <<>>).

%% From a comma separated string
get_atom_list(Key, L) ->
	case proplists:get_value(Key, L) of
		undefined ->
			[];
		Bin ->
			split_bin_to_atoms(Bin, <<", ">>)
	end.

split_bin_to_atoms(Subject, Pattern) ->
	split_bin_to_atoms0(binary:split(Subject, Pattern), Pattern, []).

split_bin_to_atoms0([<<>>], _, Acc) ->
	lists:reverse(Acc);
split_bin_to_atoms0([B], _, Acc) ->
	lists:reverse([list_to_atom(binary_to_list(B))|Acc]);
split_bin_to_atoms0([<<>>, Rest], Pattern, Acc) ->
	split_bin_to_atoms0(binary:split(Rest, Pattern), Pattern, Acc);
split_bin_to_atoms0([B, Rest], Pattern, Acc) ->
	At = list_to_atom(binary_to_list(B)),
	split_bin_to_atoms0(binary:split(Rest, Pattern),
		Pattern, [At|Acc]).

binary_to_number(B) ->
    list_to_number(binary_to_list(B)).

list_to_number(L) ->
    try list_to_float(L)
    catch
        error:badarg ->
            list_to_integer(L)
    end.


