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

-ifdef(TEST).
-include_lib("eunit/include/eunit.hrl").
-endif.

-behavior(gen_server).

%% API
-export([start/0, stop/0,
	get_last_poll_time/0]).

%% gen_server callbacks
-export([init/1, handle_call/3, handle_cast/2, handle_info/2,
         terminate/2, code_change/3]).

-record(state, {timer, last_poll_time}).
-define(SERVER, ?MODULE).

-include_lib("OpenACD/include/log.hrl").
-include_lib("OpenACD/include/cpx.hrl").
-include_lib("OpenACD/include/queue.hrl").
-include_lib("OpenACD/include/call.hrl").
-include_lib("OpenACD/include/agent.hrl").

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
	spx_db:connect(),	
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

process_skill(Skill, "ADD") ->
	call_queue_config:new_skill(
		get_atom(<<"atom">>, Skill),
		get_str(<<"name">>, Skill),
		get_str(<<"description">>, Skill),
		get_str(<<"groupName">>, Skill));

process_skill(Skill, "DELETE") ->
	call_queue_config:destroy_skill(
		get_str(<<"name">>, Skill));

process_skill(Skill, "UPDATE") ->
	call_queue_config:set_skill(
		get_atom(<<"atom">>, Skill),
		get_str(<<"name">>, Skill),
		get_str(<<"description">>, Skill),
		get_str(<<"groupName">>, Skill));

process_skill(_, Command) ->
	?WARNING("Unrecognized command: ~s", [Command]),
	{error, unkown_command}.

process_client(Client, "ADD") ->
	call_queue_config:new_client(
		get_str(<<"name">>, Client),
		get_str(<<"identity">>, Client),
		get_client_options(Client));

process_client(Client, "DELETE") ->
	call_queue_config:destroy_client(
		get_str(<<"identity">>, Client));

process_client(Client, "UPDATE") ->
	call_queue_config:set_client(
		get_str(<<"identity">>, Client),
		get_str(<<"name">>, Client),
		get_client_options(Client));

process_client(_, Command) ->
	?WARNING("Unrecognized command: ~s", [Command]).

process_queue_group(QueueGroup, "ADD") ->
	NewQgroup = #queue_group{
		name = get_str(<<"name">>, QueueGroup),
		sort = 10,
		recipe = [],
		skills = get_all_skills(QueueGroup)},
	call_queue_config:new_queue_group(NewQgroup);
process_queue_group(QueueGroup, "DELETE") ->
	call_queue_config:destroy_queue_group(
		get_str(<<"name">>, QueueGroup));
process_queue_group(QueueGroup, "UPDATE") ->
	OldName = get_str(<<"oldName">>, QueueGroup),
	%% warn: this is not atomic
	case call_queue_config:get_queue_group(OldName) of
		{atomic, []} ->
			?WARNING("Queue group ~s not found!", [OldName]);
		{atomic, [OldQueueGroup]} ->
			NewQueueGroup = OldQueueGroup#queue_group{
				name = get_str(<<"name">>, QueueGroup),
				sort = 10,
				skills = get_all_skills(QueueGroup)
			},
			call_queue_config:set_queue_group(OldName, NewQueueGroup)
	end;
process_queue_group(_, Command) ->
	?WARNING("Unrecognized command: ~s", [Command]),
	{error, unkown_command}.

process_queue(Queue, "ADD") ->
	Name = get_str(<<"name">>, Queue),

	call_queue_config:new_queue(
		Name,
		get_str_to_int(<<"weight">>, Queue),
		get_all_skills(Queue),
		get_recipes(Queue),
		get_str(<<"queueGroup">>, Queue)
	),
	queue_manager:load_queue(Name);

process_queue(Queue, "DELETE") ->
	call_queue_config:destroy_queue(
		get_str(<<"name">>, Queue));

process_queue(Queue, "UPDATE") ->
	Name = get_str(<<"name">>, Queue),
	call_queue_config:set_queue(
		get_str(<<"oldName">>, Queue),
		Name,
		get_str_to_int(<<"weight">>, Queue),
		get_all_skills(Queue),
		get_recipes(Queue),
		get_str(<<"queueGroup">>, Queue)),
	queue_manager:load_queue(Name);

process_queue(_, Command) ->
	?WARNING("Unrecognized command: ~s", [Command]),
	{error, unkown_command}.

process_fs_media_manager(Config, _Command) ->
	%% TODO should be getting a boolean than a string
	case get_bin(<<"enabled">>, Config) of
		<<"true">> ->
			cpx_supervisor:update_conf(freeswitch_media_manager,
				#cpx_conf{
					id = freeswitch_media_manager,
					module_name = freeswitch_media_manager,
					start_function = start_link,
					start_args = [
						get_atom(<<"node">>, Config),
						[{h323,[]}, {iax2,[]}, {sip,[]},
						 {dialstring,
						 get_str(<<"dialString">>, Config)}]],
					supervisor = mediamanager_sup
				});
		<<"false">> ->
			cpx_supervisor:destroy(freeswitch_media_manager);
		_ ->
			?WARNING("Unrecognized fs_media_manager state", [])
	end.

process_agent_configuration(Config, _Command) ->
	%% TODO should be boolean than string
	case get_bin(<<"listenerEnabled">>, Config) of
		<<"true">> ->
			cpx_supervisor:update_conf(agent_dialplan_listener,
				#cpx_conf{id = agent_dialplan_listener,
					module_name = agent_dialplan_listener,
					start_function = start_link,
					start_args = [],
					supervisor = agent_connection_sup});
		<<"false">> ->
			cpx_supervisor:destroy(agent_dialplan_listener);
		_ ->
			?WARNING("Unrecognized agent_configuration state", [])
	end.

process_log_configuration(Config, _Command) ->
	LogLevel = get_atom(<<"logLevel">>, Config),
	LogDir = get_str(<<"logDir">>, Config),

	FullLogPath = filename:join(LogDir, "full.log"),
	ConsoleLogPath = filename:join(LogDir, "console.log"),

	cpxlog:set_loglevel(FullLogPath, LogLevel),
	cpxlog:set_loglevel(ConsoleLogPath, LogLevel).

extract_condition(MongoCondition) ->
	Condition = get_atom(<<"condition">>, MongoCondition),
	Relation = get_atom(<<"relation">>, MongoCondition),

	case Condition of
		client ->
			{client, Relation, get_str(<<"value">>, MongoCondition)};
		type ->
			{type, Relation, get_atom(<<"value">>, MongoCondition)};
		ticks ->
			{ticks, get_str_to_int(<<"value">>, MongoCondition)};
		_ -> %% TODO would probably best to enumerate
			{Condition, Relation, get_str_to_int(<<"value">>, MongoCondition)}
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
	ActionProps = proplists:get_value(<<"action">>, RecipeStep),
	{array, ConditionArr} = proplists:get_value(<<"conditions">>, RecipeStep),
	Frequency = get_atom(<<"frequency">>, RecipeStep),
	StepName = get_str(<<"stepName">>, RecipeStep),

	%% TODO should be a list
	Action = get_atom(<<"action">>, ActionProps),
	ActionValue = case Action of
		announce ->
			get_str(<<"actionValue">>, ActionProps);
		set_priority ->
			get_str_to_int(<<"actionValue">>, ActionProps);
		add_skills ->
			get_atom_list(<<"actionValue">>, ActionProps);
		remove_skills ->
			get_atom_list(<<"actionValue">>, ActionProps);
		_ ->
			[]
	end,

	ActionTuple = {Action, ActionValue},
	Conditions = [extract_condition(X) || X <- ConditionArr],

	{Conditions, [ActionTuple], Frequency, StepName}.
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

get_client_options(Client) ->
	Options = proplists:get_value(<<"additionalObjects">>, Client, []),

	lists:foldl(
		fun({<<"vm_priority_diff">>, N}, Acc) when is_float(N) ->
			[{vm_priority_diff, trunc(N)}|Acc];
		(Any, Acc) ->
			?WARNING("Not saving unknown client option: ~p", [Any]),
			Acc
		end, [], Options).

get_recipes(Queue) ->
	{array, RecipeStepsJ} = proplists:get_value(
		<<"additionalObjects">>, Queue),

	[extract_recipe_step(X) || X <- RecipeStepsJ].

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

%% From a comma separated string
get_atom_list(Key, L) ->
	case proplists:get_value(Key, L) of
		undefined ->
			[];
		Bin when is_binary(Bin)->
			split_bin_to_atoms(Bin, <<", ">>)
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
