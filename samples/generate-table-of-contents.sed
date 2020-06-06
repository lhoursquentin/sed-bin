#!/bin/sed -f

# Generate table of contents with links for markdown files
# Usage: sed -f <this-script> <mardown file>

# ignore code blocks
/^```/,/^```/d

# no need to index ourselves
/^# Table of contents/d

# found heading
/^#/{
  # save our line and first work on the actual URI
  h
  # strip leading blanks
  s/^#*[[:blank:]]*//
  s/[[:blank:]]/-/g
  # punctuation and anything funky gets lost
  s/[^-[:alnum:]]//g
  # swap with hold and work on the displayed title
  x
  # get rid of last leading # and potential white spaces
  s/^\(#\)*#[[:blank:]]*/\1/
  # the remaining leading # (if any) will be used for indentation
  s/#/  /g
  # prepare the first half of the markdown
  s/\( *\)\(.*\)/\1* [\2](#/
  # append the link kept and remove the newline
  G
  s/\(.*\)[[:space:]]\(.*\)/\1\2)/p
}
d
