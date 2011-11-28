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
-export([start/0, stop/0]).

%% gen_server callbacks
-export([init/1, handle_call/3, handle_cast/2, handle_info/2,
         terminate/2, code_change/3]).

-record(state, {}).
-define(SERVER, ?MODULE).

-include("log.hrl").
-include("cpx.hrl").
-include("queue.hrl").

start() ->
	gen_server:start_link({local, ?SERVER}, ?SERVER, [], []).

stop() ->
	gen_server:call(?SERVER, stop).


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
	{ok, start_poller()}.

%--------------------------------------------------------------------
%% Function: %% handle_call(Request, From, State) -> {reply, Reply, State} |
%%                                      {reply, Reply, State, Timeout} |
%%                                      {noreply, State} |
%%                                      {noreply, State, Timeout} |
%%                                      {stop, Reason, Reply, State} |
%%                                      {stop, Reason, State}
%% Description: Handling call messages
%%--------------------------------------------------------------------
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

start_poller() ->
	mongodb:singleServer(def),
	mongodb:connect(def),
	init_poller(10000).

% @doc Spawn a poller process
%
% @spec init(PollInterval::integer())
init_poller(PollInterval) ->
    spawn_link(
    	fun() -> loop(PollInterval, 0) end
	).

loop(PollInterval, LastPollTime) ->
	receive
		exit -> ok
	after
		PollInterval ->
			{NewPollTime} =
				get_new_config(LastPollTime),
			loop(PollInterval, NewPollTime)
    end.

get_new_config(_LastPollTime) ->
	NewPollTime = calendar:datetime_to_gregorian_seconds(
		{ date(), time() }
	),
	%connect to openacd db and count objects in commands collection
	Mong = mongoapi:new(def,<<"openacd">>),
	CommandCount = Mong:count("commands"),
	if CommandCount =:= 0 -> ?DEBUG("No Command to execute", []);
		true ->
			%if command count > 0 retrieve all commands and process them
			?WARNING("No of Commands to execute ~p", [CommandCount]),
			{_Status, Commands} = Mong:find("commands", [], undefined, 0, CommandCount),
			lists:foreach(fun(Cmd) ->
				get_command_values(Cmd, Mong)
			end, Commands)
	end,
    { NewPollTime }.

get_command_values(Data, Mong) ->
	if Data =:= [] -> ?DEBUG("No Data", []);
		true ->
			% command format { "_id" : ObjectId("4ce62e892957ca4fc97387a1"), "command" : "ADD", "count" : 2, "objects" : []}
			?DEBUG("Processing Mongo DB Command: ~p", [Data]),
			[{_, Id}, {_, CmdValue}, {_, _Count}, {_, {_, Objects}}] = Data,
			lists:foreach(fun(Object) ->
				% objects to process starts with type e.g. "type" : "agent", "name" : "bond", "pin" : "1234"
				{_, Type} = lists:nth(1, Object),
				if Type =:= <<"agent">> ->
					process_agent(Object, erlang:binary_to_list(CmdValue));
				Type =:= <<"profile">> ->
					process_profile(Object, erlang:binary_to_list(CmdValue));
				Type =:= <<"skill">> ->
					process_skill(Object, erlang:binary_to_list(CmdValue));
				Type =:= <<"client">> ->
					process_client(Object, erlang:binary_to_list(CmdValue));
				Type =:= <<"queueGroup">> ->
					process_queue_group(Object, erlang:binary_to_list(CmdValue));
				Type =:= <<"queue">> ->
					process_queue(Object, erlang:binary_to_list(CmdValue));
				Type =:= <<"freeswitch_media_manager">> ->
					process_fs_media_manager(Object, erlang:binary_to_list(CmdValue));
				Type =:= <<"agent_configuration">> ->
					process_agent_configuration(Object, erlang:binary_to_list(CmdValue));
				Type =:= <<"log_configuration">> ->
					process_log_configuration(Object, erlang:binary_to_list(CmdValue));
				Type =:= <<"vm_priority_diff">> ->
					process_vm_priority_diff(Object, erlang:binary_to_list(CmdValue));
				true -> ?WARNING("Unrecognized type", [])
				end
			end, Objects),
			Mong:runCmd([{"findandmodify", "commands"},{"query", [{"_id",Id}]},{"remove",1}])
	end.

process_agent(Agent, Command) ->
	{_, Name} = lists:nth(2, Agent),
	{_, Pin} = lists:nth(3, Agent),
	{_, Group} = lists:nth(4, Agent),
	{_, Skills} = lists:nth(5, Agent),
	{_, Queues} = lists:nth(6, Agent),
	{_, Clients} = lists:nth(7, Agent),
	{_, Firstname} = lists:nth(8, Agent),
	{_, Lastname} = lists:nth(9, Agent),
	{_, Security} = lists:nth(11, Agent),
	SkillsList = lists:flatmap(fun(X)->[list_to_atom(X)] end, string:tokens((erlang:binary_to_list(Skills)), ", ")),
	QueuesList = lists:flatmap(fun(X)->[{'_queue',X}] end, string:tokens((erlang:binary_to_list(Queues)), ", ")),
        ClientsList = lists:flatmap(fun(X)->[{'_brand',X}] end, string:tokens((erlang:binary_to_list(Clients)), ", ")),
        AllSkills = lists:merge(lists:merge(QueuesList, ClientsList), SkillsList),
	if Security =:= <<"SUPERVISOR">> ->
		SecurityAtom = supervisor;
	Security =:= <<"ADMIN">> ->
		SecurityAtom = admin;
	true -> SecurityAtom = agent
	end,
	if Command =:= "ADD" ->
		agent_auth:add_agent(erlang:binary_to_list(Name), erlang:binary_to_list(Firstname), erlang:binary_to_list(Lastname), erlang:binary_to_list(Pin), AllSkills, SecurityAtom, erlang:binary_to_list(Group));
	Command =:= "DELETE" ->
		agent_auth:destroy(erlang:binary_to_list(Name));
	Command =:= "UPDATE" ->
		{_, Oldname} = lists:nth(10, Agent),
		{_, [Old]} = agent_auth:get_agent(erlang:binary_to_list(Oldname)),
		agent_auth:set_agent(element(2, Old), erlang:binary_to_list(Name), erlang:binary_to_list(Pin), AllSkills, SecurityAtom, erlang:binary_to_list(Group), erlang:binary_to_list(Firstname), erlang:binary_to_list(Lastname));
	true -> ?WARNING("Unrecognized command", [])
	end.

process_profile(Profile, Command) ->
	{_, Name} = lists:nth(2, Profile),
	{_, Skills} = lists:nth(3, Profile),
        {_, Queues} = lists:nth(4, Profile),
        {_, Clients} = lists:nth(5, Profile),
        SkillsList = lists:flatmap(fun(X)->[list_to_atom(X)] end, string:tokens((erlang:binary_to_list(Skills)), ", ")),
	QueuesList = lists:flatmap(fun(X)->[{'_queue',X}] end, string:tokens((erlang:binary_to_list(Queues)), ", ")),
	ClientsList = lists:flatmap(fun(X)->[{'_brand',X}] end, string:tokens((erlang:binary_to_list(Clients)), ", ")),
	AllSkills = lists:merge(lists:merge(QueuesList, ClientsList), SkillsList),
	if Command =:= "ADD" ->
		agent_auth:new_profile(erlang:binary_to_list(Name), AllSkills);
	Command =:= "DELETE" ->
		agent_auth:destroy_profile(erlang:binary_to_list(Name));
	Command =:= "UPDATE" ->
		{_, Oldname} = lists:nth(6, Profile),
		_Old = agent_auth:get_profile(erlang:binary_to_list(Oldname)),
		agent_auth:set_profile(erlang:binary_to_list(Oldname), erlang:binary_to_list(Name), AllSkills);
	true -> ?WARNING("Unrecognized command", [])
	end.

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

binary_to_number(B) ->
    list_to_number(binary_to_list(B)).

list_to_number(L) ->
    try list_to_float(L)
    catch
        error:badarg ->
            list_to_integer(L)
    end.

