%% Copyright (c) 2012 eZuce, Inc. All rights reserved.

-module(spx_db).

-export([connect/0]).

-spec(connect() -> ok).
connect() ->
	ReplHosts = case application:get_env(mongo_hostport) of
		undefined -> ["127.0.0.1:27017"];
		H -> H
	end,

	mongodb:replicaSets(spx, ReplHosts),
	mongodb:connect(spx).
