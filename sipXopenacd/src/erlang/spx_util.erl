-module(spx_util).

-export([
    get_str/2,
    get_atom/2,
    get_bin/2,
    get_str_to_int/2,
    get_int/2
]).

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