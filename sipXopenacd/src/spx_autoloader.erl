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

-module(spx_autoloader).
-author("eZuce").

-include_lib("OpenACD/include/log.hrl").
-include_lib("OpenACD/include/call.hrl").
-include_lib("OpenACD/include/cpx.hrl").
-include_lib("OpenACD/include/agent.hrl").

%% API
-export([
	start/0,
	start/1,
	start_link/0,
	start_link/1,

	autoload/0,
	add_mod/2,
	get_interval/0,
	get_states/0,

	stop/0
]).

%% gen_server callbacks
-export([
	init/1,
	handle_call/3,
	handle_cast/2,
	handle_info/2,
	terminate/2,
	code_change/3]).

-define(DEFAULT_INTERVAL, 10000).

-record(state, {
	interval = ?DEFAULT_INTERVAL :: non_neg_integer() | none,
	timer, %% timer()
	last_load,
	last_tick,
	mods = []
}).

-ifdef(TEST).
-include_lib("eunit/include/eunit.hrl").
-endif.

-ifdef(TEST).
-define(DB, <<"imdb_test">>).
-else.
-define(DB, <<"imdb">>).
-endif.

-define(COLL, <<"entity">>).

%% @doc Start linked with given options.
-spec(start_link/1 :: (Options :: any()) -> {'ok', pid()}).
start_link(Options) ->
    gen_server:start_link({local, ?MODULE}, ?MODULE, Options, []).

%% @doc start linked with default options.
-spec(start_link/0 :: () -> {'ok', pid()}).
start_link() ->
	start_link([]).

%% @doc Start unlinked.
-spec(start/1 :: (Options :: any()) -> {'ok', pid()}).
start(Options) ->
	gen_server:start({local, ?MODULE}, ?MODULE, Options, []).

%% @doc Start unlinked with default options.
-spec(start/0 :: () -> {'ok', pid()}).
start() ->
	start([]).

stop() ->
	gen_server:call(?MODULE, stop).

autoload() ->
	gen_server:call(?MODULE, autoload).

add_mod(Mod, InitModState) ->
	gen_server:cast(?MODULE, {add_mod, Mod, InitModState}).

get_interval() ->
	gen_server:call(?MODULE, get_interval).

get_states() ->
	gen_server:call(?MODULE, get_states).

%set_interval() ->

%%====================================================================
%% gen_server callbacks
%%====================================================================

%%--------------------------------------------------------------------
%% Function: init(Args) -> {ok, State} |
%%--------------------------------------------------------------------
init(Opts) ->
	add_autoloads(),

	Interval = proplists:get_value(interval, Opts, ?DEFAULT_INTERVAL),

	Timer = timer:send_interval(Interval, ping),

	self() ! ping,
	{ok, #state{
		timer = Timer,
		interval = Interval
	}}.

%--------------------------------------------------------------------
%% Function: %% handle_call(Request, From, State) -> {reply, Reply, State} |
%%                                      {reply, Reply, State, Timeout} |
%%                                      {noreply, State} |
%%                                      {noreply, State, Timeout} |
%%                                      {stop, Reason, Reply, State} |
%%                                      {stop, Reason, State}
%% Description: Handling call messages
%%--------------------------------------------------------------------
handle_call(autoload, _From, State) ->
	State1 = autoload(State),
	{reply, ok, State1};
handle_call(get_interval, _From, State) ->
	{reply, State#state.interval, State};
handle_call(get_states, _From, State) ->
	{reply, State#state.mods, State};
handle_call(stop, _From, State) ->
	{stop, normal, ok, State};
handle_call(_Request, _From, State) ->
    Reply = invalid,
    {reply, Reply, State}.

%%--------------------------------------------------------------------
%% Function: handle_cast(Msg, State) -> {noreply, State} |
%%--------------------------------------------------------------------
handle_cast({add_mod, Mod, InitModState}, State) ->
	{Name, _, _, _, _} = Mod,
	Mods = State#state.mods,
	%% TODO Add name and base it from there
	NState = case lists:member(Name, [X || {{X, _, _, _}, _} <- Mods]) of
		true ->
			ok; %% TODO change instead of ignore
		false ->
			State#state{mods = [{Mod, InitModState}|Mods]}
	end,
	{noreply, NState};
handle_cast(_Msg, State) ->
    {noreply, State}.

%%--------------------------------------------------------------------
%% Function: handle_info(Info, State) -> {noreply, State} |
%%--------------------------------------------------------------------
handle_info(ping, State) ->
	State1 = autoload(State),
    {noreply, State1}.


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

autoload(State) ->
	NewMods = lists:map(
		fun({Mod = {Name, ActionFun, LoadFun, UnloadFun, ReloadFun}, Conf}) ->
			%?DEBUG("Checking autoload of ~p with config: ~p", [Name, Conf]),
			NewConf = case catch ActionFun(Conf) of
				{load, NConf} ->
					try_do(Name, load, LoadFun, Conf, NConf);
				{unload, NConf} ->
					try_do(Name, unload, UnloadFun, Conf, NConf);
				{reload, NConf} ->
					try_do(Name, reload, ReloadFun, Conf, NConf);
				none ->
					Conf;
				{'EXIT', Err} ->
					?WARNING("Error occured getting autoload action: ~p", [Err]),
					Conf
			end,
			{Mod, NewConf}
		end, State#state.mods),
	State#state{mods = NewMods}.

try_do(Name, Action, Fun, Conf, NConf) ->
	?DEBUG("~p doing ~p with ~p", [Name, Action, NConf]),
	case catch Fun(NConf) of
		{'EXIT', Err} ->
			?WARNING("Error occured while ~p ~p: ~p", [Name, Action, Err]),
			Conf;
		_ ->
			?INFO("~p did ~p", [Name, Action]),
			NConf
	end.

add_autoloads() ->
	spx_agentconfig_loader:start(),
	spx_log_loader:start(),
	spx_freeswitchmedia_loader:start(),
	spx_webmgt_loader:start(),
	spx_agentweb_loader:start(),
	spx_queue_loader:start(),

	ok.

-ifdef(TEST).

add_mod_with_action(A) ->
	LoadFun = fun(X) -> fke:load(X) end,
	UnloadFun = fun(X) -> fke:unload(X) end,
	ReloadFun = fun(X) -> fke:reload(X) end,

	ActionFun = case is_function(A) of
		true -> A;
		_ -> fun(_) -> A end
	end,
	ModState = 1,
	add_mod({fakemod, ActionFun, LoadFun, UnloadFun, ReloadFun}, ModState).

ping_test_() ->
	{foreach,
		fun() ->
			spx_autoloader:start_link(),

			meck:new(fke),
			meck:expect(fke, load, fun(_) -> ok end),
			meck:expect(fke, unload, fun(_) -> ok end),
			meck:expect(fke, reload, fun(_) -> ok end)
		end,
		fun(_) ->
			stop(),
			meck:unload(fke)
		end,
		[fun() ->
			add_mod_with_action(none),
			autoload(),

			?assertEqual([], meck:history(fke))
		end,
		fun() ->
			add_mod_with_action({load, 10}),
			autoload(),

			?assert(meck:validate(fke)),
			?assert(meck:called(fke, load, [10]))
		end,
		fun() ->
			add_mod_with_action({unload, 10}),
			autoload(),

			?assert(meck:validate(fke)),
			?assert(meck:called(fke, unload, [10]))
		end,
		fun() ->
			add_mod_with_action({reload, 10}),

			autoload(),

			?assert(meck:validate(fke)),
			?assert(meck:called(fke, reload, [10]))
		end,
		fun() ->
			add_mod_with_action(fun(X) -> {load, X+1} end),

			autoload(),
			autoload(),

			?assert(meck:validate(fke)),
			?assert(meck:called(fke, load, [2])),
			?assert(meck:called(fke, load, [3]))
		end
		]
	}.

-endif.
