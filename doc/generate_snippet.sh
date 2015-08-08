#!/bin/bash
# Auto generate snippet from a test file given in parameter

CURRENT_FUNCTION_NAME=""
N_OPEN=0
SNIPPING=0
TARGET="snippet/""`basename "$1"`"

cd "`dirname "$1"`"
mkdir -p snippet
touch "$TARGET"


while IFS='' read -r line || [[ -n "$line" ]]; do
    if [ $SNIPPING -eq 0 ]; then
            # Not currently snipping a function
        if [ "`echo "$line" | grep -E "^void\ ?\*?.*?\)"`" != "" ]; then
            # Found a function
            SNIPPING=1
            CURRENT_FUNCTION_NAME="`expr "$line" : "^void \?\*\? \?\(.*\?\)(.*\?)"`"
            echo "//! [$CURRENT_FUNCTION_NAME]" >> "$TARGET"
        fi
    fi;
    if [[ "$line" == *"{"* ]]; then
        countopen=${line//[^\{]/}
        let "N_OPEN+=${#countopen}" # Add number of open
    fi
    if [[ "$line" == *"}"* ]]; then
        countclose=${line//[^\}]/}
        let "N_OPEN-=${#countclose}" # Add number of close
    fi
    #    if [[ "$line" != *"assert"*  ]]; then #.*?"assert".*?"(".*?")".*
    #        echo "$line" >> "$TARGET"
    #    else
    #        echo "${line/assert/// assert}" >> "$TARGET"
    #    fi
    echo "$line" >> "$TARGET"
    if [ $N_OPEN -eq 0 ] && [ $SNIPPING -eq 1 ]; then
        echo "//! [$CURRENT_FUNCTION_NAME]" >> "$TARGET"
        SNIPPING=0
    fi
done < "$1"

exit 0