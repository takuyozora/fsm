#!/bin/bash

CURRENT_FUNCTION_NAME=""
CURRENT_FUNCTION_OPEN=0
SNIPPING=0
#ALREADY_SNIPPED=0

cd "`dirname "$1"`"
TARGET="snippet/""`basename "$1"`"

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
    countopen=${line//[^\{]/}
    countclose=${line//[^\}]/}
    if [[ "$line" == *"{"* ]]; then
    let "CURRENT_FUNCTION_OPEN+=${#countopen}" # Add number of open
    fi
    if [[ "$line" == *"}"* ]]; then
    let "CURRENT_FUNCTION_OPEN-=${#countclose}" # Add number of close
    fi
    if [[ "$line" != *"assert"*  ]]; then #.*?"assert".*?"(".*?")".*
        echo "$line" >> "$TARGET"
    else
        echo "${line/assert/// assert}" >> "$TARGET"
    fi
    if [ $CURRENT_FUNCTION_OPEN -eq 0 ] && [ $SNIPPING -eq 1 ]; then
        echo "//! [$CURRENT_FUNCTION_NAME]" >> "$TARGET"
        SNIPPING=0
    fi
done < "$1"


exit 0