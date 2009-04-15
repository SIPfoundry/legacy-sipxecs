#! /bin/bash

# Check all files for correct use of tabs.

# The rule to be enforced is:
#    All files that have svn:mime-type starting with "text/" must
#    limit their use of tabs based on their property sf:tab:
#        yes - tabs are allowed in the file
#        initial - tabs are allowed as initial character(s) of lines
#        no or not set - tabs are not allowed

# Arguments are a list of files or directories in a Subversion working
# copy to check for violations.  Default argument is ".".

T=${TMPDIR:-/tmp}/$$tab-check

# Set default argument of "." if necessary.
if [[ $# -eq 0 ]]
then
    set .
fi

# Make the list of files with text media types.

# List all the files with svn:mime-type properties.
svn propget svn:mime-type -R "$@" >$T.temp1
# Get the text files.
sed <$T.temp1 \
    -e 's@ - text/[^ ][^ ]*$@@' \
    -e '/ - /d' |
sort >$T.text

# Make the lists of files according to sf:tab.

# List all the files with sf:tab properties.
svn propget sf:tab -R "$@" >$T.temp2
# Get the files with sf:tab = yes
sed <$T.temp2 \
    -e '/ - yes$/!d' \
    -e 's@ - yes$@@' |
sort >$T.yes
# Get the files with sf:tab = initial
sed <$T.temp2 \
    -e '/ - initial$/!d' \
    -e 's@ - initial$@@' |
sort >$T.initial
# Get the files with sf:tab = no (all others)
comm -23 $T.text $T.yes | comm -23 - $T.initial >$T.no

echo TEXT:
cat $T.text
echo

echo YES:
cat $T.yes
echo

echo INITIAL:
cat $T.initial
echo

echo NO:
cat $T.no
echo

# Report violations for sf:tab = no.

<$T.no xargs grep --files-with-match $'\t' >$T.no-violations

echo "----- The following files contain tabs but sf:tab = no:"
cat $T.no-violations
echo "----- [end]"
echo

# Report violations for sf:tab = initial.
# an svn:eol-style.

<$T.initial xargs grep --files-with-match $'^.*[^\\n\t#].*\t' >$T.initial-violations

echo "----- The following files contain non-initial tabs but sf:tab = initial:"
cat $T.initial-violations
echo "----- [end]"
echo

# Return success only if all lists of violations are empty.
# Record the return status we want.
[[ ! -s $T.no-violations ]] && \
    [[ ! -s $T.initial-violations ]]
STATUS=$?

# Delete the temporary files
rm -f $T.temp1 $T.text $T.temp2 $T.yes $T.initial $T.no \
    $T.no-violations $T.initial-violations

exit $STATUS
