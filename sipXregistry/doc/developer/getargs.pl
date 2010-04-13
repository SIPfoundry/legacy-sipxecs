#!perl
###
### (c) Copyright 1992,1993,1994,1995 Scott D. Lawrence
###     All rights reserved.
###
### Permission is hereby granted to use, copy, and distribute this code
### in any way provided that the above copyright is included in all copies.
### No warranty is of suitability for any purpose is provided by the author.
###
### [That having been said, if you find a problem in this package (or use
###  it and don't find a problem) I would love to hear from you:
###     scott (at) skrb (dot) org
###
### This package provides the routine 'getargs' for parsing command line
### arguments.  It automates most of a standard command line interface by
### taking a picture of the expected arguments of the form:
###
###     ( <4tuple> [, <4tuple> ]... )
###
### <4tuple> ::= <type>, <keyword>, <size>, <variable>
###
### <type>   ::= '-' for switch arguments (these are order independent
###                      among themselves, but must all appear before any
###                      positional arguments)
###              'm' for mandatory positional arguments
###              'o' for optional positional arguments
###              'h' provides the help text; keyword and size are ignored.
###                      The contents of the scalar named in the variable
###                      are saved to print in the usage message.  This may
###                      appear anywhere in the argument picture, but only
###                      one may be specified.
###
### <keyword>  ::= string to match for switch arguments
###                (also used to print for usage of postional arguments)
###
###                To provide a short form and long form for an argument,
###                use '<short>|<long>'; the long form will be recognized
###                only if preceded by a double dash.  For example:
###                '-', 'f|force' specifies the switches '-f' and '--force'
###                as being equivalent.
###
### <size>     ::= number of values to consume from ARGV
###                0 ::= increment variable using '++'
###                      (used for flag switches)
###               >1 ::= set list variable to next 'n' values
###               -1 ::= set list variable to remaining values
###                      (for switch arguments, the values following
###                       '--' are not consumed)
###
### <variable> ::= name of variable (not including $ or @) to assign
###                argument value into
###
### Provides -usage, --usage, -help, --help, and -?
###      (if the 'usage' or 'help' switches are specified in the picture,
###       the caller will get it like any other switch).
###
### Provides '--' for the end of switch arguments.
###
### Switch and Optional arguments not specified in @ARGV are not
### defined by getargs - you can either test for that or just assign
### them default values before calling getargs.
###
### @ARGV is not modified.
###    The getargs routine can be used for interactive command parsing
###    by reading the command, splitting the results into @ARGV, and
###    calling getargs as you would for the real command line.
###
### Returns 1 if @ARGV parsed correctly according to the picture; if not,
### it prints the usage message and returns 0;
###
### Example:
###
###   $HelpText = <<_HELP_TEXT_;
###   This is the help text for this command.
###   _HELP_TEXT_
###
###   &getargs( '-', 'flag',        0, 'Flag'
###            ,'-', 'value',       1, 'Value'
###            ,'-', 'list',        2, 'List'
###            ,'-', 'a|alternate', 1, 'Alternate'
###            ,'-', 'values',     -1, 'Values'
###            ,'m', 'mandatory',   1, 'Mandatory'
###            ,'m', 'mandatory2',  1, 'Mandatory2'
###            ,'o', 'optional',    1, 'Optional'
###            ,'h', '',            0, 'HelpText'
###            ) || exit;
###
###    Produces the usage picture:
###
################################################################
###
###    testargs
###             [-flag]
###             [-value <value>]
###             [-list <list> <list>]
###             [-a|--alternate <alternate>]
###             [-values <values> ... ]
###             [--]
###             <mandatory>
###             <mandatory2>
###             [<optional>]
###
###    This is the help text for this command.
################################################################
###
###    and sets the variables: $Mandatory, $Mandatory2
###    and (if specified):     $Flag, $Value, @List, $Alternate,
###                            @Values, $Optional
###

package getargs;

sub main'getargs #'
{
    local(@Picture) = @_;

    # Now  parse the argument picture
    local( $Type, $Keyword, $Key, $Size, $Variable, $Tuple );
    local( %Sizes, %Switches );
    local( $Options, $Mandatories, @Positional, $Target, %Targets );

    for ( $Tuple = 0; $Tuple < $#Picture; $Tuple += 4 )
    {
        ( $Type, $Keyword, $Size, $Variable ) = @Picture[ $Tuple..$Tuple+3 ];

        if ( $Keyword =~ /^([a-zA-Z])\|([a-zA-Z]+[-a-zA-Z]*)$/ )
        {
            die "Only switch keywords may have alternate values ('|')"
                if ( $Type ne '-' );

            local( $ShortKey, $LongKey ) = ( $1, "-$2" );

            $Sizes{ $ShortKey } = $Size;
            $Targets{ $ShortKey } = $Variable;

            $Sizes{ $LongKey } = $Size;
            $Targets{ $LongKey } = $Variable;
        }
        elsif ( $Type ne 'h' )
        {
            $Sizes{ $Keyword } = $Size;
            $Targets{ $Keyword } = $Variable;
        }

        if ( $Type eq '-' ) # switch argument
        {
            die "Switch Argument specified after Positionals\n"
                if ( $Options || $Mandatories )
                }
        elsif ( $Type eq 'm' ) # mandatory positional argument
        {
            die "Optional Arg in picture before Mandatory Arg\n"
                if $Options;
            $Mandatories++;
            push( @Positional, $Keyword );
        }
        elsif ( $Type eq 'o' ) # optional positional argument
        {
            $Options++;
            push( @Positional, $Keyword );
        }
        elsif ( $Type eq 'h' ) # help message argument
        {
            defined( $HelpText )
                && die "Only one help text parameter ('h') allowed.\n";

            $Assignment = '$HelpText = $main\''.$Variable.';';
            eval $Assignment;
            $HelpText || die "Help text specified in $Variable is empty\n";
        }
        else
        {
            die "Undefined Type: $Type\n";
        }
    }

    ###
    ### Parse Switch Arguments from Actual Argument List
    ###

    local( @ActualArgs ) = @ARGV;

  Switch:
    while ( $#Switches && ($_ = shift @ActualArgs) )
    {
        if ( /^--$/ ) ## force end of options processing
        {
            #print "END OPTIONS\n"; #<DEBUG
            last Switch;
        }
        elsif ( /^-\?$/ )
        {
            &usage( @Picture );
            return 0;
        }
        elsif ( /^-\d+/ ) ## numeric argument - not an option
        {
            unshift( @ActualArgs, $_ );
            last Switch;
        }
        elsif ( s/^-// ) ## looks like a switch...
        {
            if ( $Target = $Targets{ $_ } )
            {
                &assign_value( $Target, $Sizes{ $_ } );
            }
            elsif ( /^-?(usage|help)$/ )
            {
                &usage( @Picture );
                return 0;
            }
            else
            {
                warn "Unknown switch -$_\n";
                &usage( @Picture );
                return 0;
            }
        }
        else
        {
            #print "END SWITCHES?\n"; #<DEBUG
            unshift( @ActualArgs, $_ );
            last Switch;
        }
    } # Switch

    ###
    ### Parse Positional Arguments from Actual Argument List
    ###

  Positional:
    while( $_ = shift( @Positional ) )
    {
        &assign_value( $Targets{ $_ }, $Sizes{ $_ } ) || last Positional;
        $Mandatories--;
    }

    if ( @ActualArgs )
    {
        warn "Too many arguments: @ActualArgs\n";
        &usage( @Picture );
        0;
    }
    elsif ( $Mandatories > 0 )
    {
        warn "Not enough arguments supplied\n";
        &usage( @Picture );
        0;
    }
    else
    {
        1;
    }

} # sub getargs

sub assign_value
{
    local ( $Target, $Size ) = @_;
    local ( $Assignment );

    if ( $Size <= @ActualArgs )
    {
        Assign:
        {
          $Assignment = '$main\''.$Target.'++;'
                                             , last Assign if ( $Size == 0 );
          $Assignment = '$main\''.$Target.' = shift @ActualArgs;'
                                             , last Assign if ( $Size == 1 );
          $Assignment =
'@main\''.$Target.' = @ActualArgs[ $[..$[+$Size-1 ],@ActualArgs = @ActualArgs[ $[+$Size..$#ActualArgs ];'
                                             , last Assign if ( $Size > 1 );
          $Assignment =
     'push( @main\''.$Target.', shift @ActualArgs )
         while ($#ActualArgs >= $[) && ($ActualArgs[$[] ne \'--\');'

                                             , last Assign if ( $Size == -1 );
          die "Invalid argument type in picture\n";
        }

        eval $Assignment;
        1;
    }
    else
    {
        @ActualArgs = ();
        0;
    }
}

sub usage
{
    local( $CommandName ) = $0;
    $CommandName =~ s\^.*/\\;
    print "Usage:\n";
    print "    $CommandName";
    local( @Picture ) = @_;
    local( $Type,  $Keyword,  $Size, $Tuple, $Switches );

    $Switches = 0;
    Switch: for ( $Tuple = 0; $Tuple < $#Picture; $Tuple += 4 )
    {
        ( $Type, $Keyword, $Size ) = @Picture[ $Tuple..$Tuple+2 ];

        if ( $Type eq "-" ) # switch argument
        {
            $Switches++;

            print "\n   "." " x length($CommandName)." ";

            if ( $Keyword =~ s/(.+)\|(.+)/$2/ )
            {
                print " [-$1|--$2";
            }
            else
            {
                print " [-$Keyword";
            }
            if ( $Size == -1 )
            {
                print " <$Keyword> ... ";
            }
            print " <$Keyword>" while ( $Size-- > 0 );
            print "]";
        }
    }

    print "\n   "." " x length($CommandName)."  [--]" if $Switches;

    Positional: for ( $Tuple = 0; $Tuple < $#Picture; $Tuple += 4 )
    {
        ( $Type, $Keyword, $Size ) = @Picture[ $Tuple..$Tuple+2 ];

        print "\n   "." " x length($CommandName)." " unless $Type eq '-';

        if ( $Type eq "m" ) # mandatory positional argument
        {
            if ( $Size == -1 )
            {
                print " <$Keyword> ...";
                last Positional;
            }
            print " <$Keyword>" while ( $Size-- > 0 );
        }
        elsif ( $Type eq "o" ) # optional positional argument
        {
            if ( $Size == -1 )
            {
                print " [<$Keyword>] ...";
                last Positional;
            }
            print " [<$Keyword>" while ( $Size-- > 0 );
            print "]";
        }
    }

    print "\n";

    defined( $HelpText ) && print $HelpText;
}
1;

### Local Variables: ***
### mode:perl ***
### End: ***
