#! /bin/bash

# Check all files for correct svn:eol-style and svn:mime-type values.

# The rule to be enforced is:
#    All files have either
#        (1) svn:mime-type starting with "text/" and an appropriate
#            svn:eol-style value, or
#        (2) svn:mime-type not starting with "text/", that
#        (3) starts with a known major type (viz.: application, audio,
#            image, message, model, multipart, text, video)
#    In addition:
#        (4) all text files must end with a proper EOL.
#        (5) text files must have no trailing whitespace on lines,
#            and no final empty lines

# Arguments are a list of files or directories in a Subversion working
# copy to check for violations.  Default argument is ".".

T=${TMPDIR:-/tmp}/eol-check.$$

# Set default argument of "." if necessary.
if [[ $# -eq 0 ]]
then
    set .
fi

# Make the list of all version-controlled files.

# List all the files and directories under the argument directories.
# This has to be done carefully, because "svn status" follows externals
# links, and "svn propget" does not.  Thus, we want to avoid listing
# included external directories.
(
    for f in "$@"
    do
      svn status -v "$f" |
      # Remove anything not under version control, and anything after
      # the "Performing status" message that indicates going into an external
      # directory.
      sed -e '/^\?/d' \
          -e '/^$/d' \
          -e '/^Performing status/,$d'
    done
) |
# Extract the file names.
cut -c 41- |
# Only keep the real files.
while read file
do
  if [ -f "$file" ]
  then
      echo "$file"
  fi
done |
sort >$T.all

#echo ALL:
#cat $T.all
#echo

# Make the lists of files with and without text media types.

# List all the files with svn:mime-type properties.
svn propget svn:mime-type -R "$@" >$T.temp1
# Get all the files with svn:mime-type.
sed <$T.temp1 \
    -e 's@ - [^ ][^ ]*$@@' |
sort >$T.mime-type
# Get the text files.
sed <$T.temp1 \
    -e 's@ - text/[^ ][^ ]*$@@' \
    -e '/ - /d' |
sort >$T.text
# Get the binary files.
sed <$T.temp1 \
    -e '/ - text\/[^ ][^ ]*$/d' \
    -e 's@ - [a-z][a-z]*/[^ ][^ ]*$@@' \
    -e '/ - /d' |
sort >$T.binary

#echo MIME-TYPE:
#cat $T.mime-type
#echo

#echo TEXT:
#cat $T.text
#echo

#echo BINARY:
#cat $T.binary
#echo

# Make the list of files with svn:eol-style properties.

# List all the files with svn:eol-style properties.
svn propget svn:eol-style -R "$@" |
# Get the text files.
sed -e 's@ - ..*$@@' \
    -e '/ - /d' |
sort >$T.eol-style

#echo EOL:
#cat $T.eol-style
#echo

# Report violations of the first rule:  Every file must have an svn:mime-type.

comm -23 $T.all $T.mime-type >$T.no-mime-type

echo "----- The following files do not have an svn:mime-type property:"
cat $T.no-mime-type
echo "----- [end]"
echo

# Report violations of the second rule:  Every text/* file must have
# an svn:eol-style.

comm -23 $T.text $T.eol-style >$T.no-eol-style

echo "----- The following text files do not have an svn:eol-style property:"
cat $T.no-eol-style
echo "----- [end]"
echo

# Report violations of the third rule:  Every svn:mime-type must be valid.

# Use an extended regexp to remove all mime types with valid major components.
# This requires the Gnu sed extension -r.
sed -r <$T.temp1 \
    -e '/ - (application|audio|image|message|model|multipart|text|video)\/[^ ][^ ]*$/d' >$T.invalid-mime-type

echo "----- The following files have an invalid svn:mime-type property:"
cat $T.invalid-mime-type
echo "----- [end]"
echo

# Report violations of the fourth rule:  Every text file must end with EOL.
echo -n $'\r' >$T.CR
echo -n $'\n' >$T.LF

while read F
do
  if [ -s "$F" ]
  then
      tail -c1 "$F" >$T.end
      if cmp -s $T.end $T.CR || cmp -s $T.end $T.LF
      then
          :
      else
          echo "$F"
      fi
  fi
done <$T.text >$T.no-eol

echo "----- The following text files do not end with EOL:"
cat $T.no-eol
echo "----- [end]"
echo

# Report violations of the fifth rule:  No text file may have
# extraneous (trailing) whitespace.

while read F
do
  if grep -q $'[ \t]$' "$F" ||
     tail -n 1 "$F" | grep -q $'^[ \t]*$'
  then
      echo "$F"
  fi
done <$T.text >$T.trailing

echo "----- The following text files have trailing whitespace:"
cat $T.trailing
echo "----- [end]"
echo

# Return success only if all lists of violations are empty.
# Record the return status we want.
[[ ! -s $T.no-mime-type ]] && \
    [[ ! -s $T.no-eol-style ]] && \
    [[ ! -s $T.invalid-mime-type ]] && \
    [[ ! -s $T.no-eol ]]
STATUS=$?

# Delete the temporary files
if true
then
    rm -f $T.all $T.binary $T.eol-style $T.invalid-mime-type $T.mime-type \
	$T.no-eol-style $T.no-mime-type $T.temp1 $T.text $T.CR $T.LF $T.end \
	$T.no-eol
fi

exit $STATUS
