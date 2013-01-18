#!/bin/bash
#
# Copyright 2013 Artem Harutyunyan, Sergey Shekyan
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

# Add command to the array
# export CMD_LIST=("${CMD_LIST[@]}" get_settings)

validate_get_users()
{
    validate_admin
    validate_curl
    validate_egrep
}

run_get_users()
{
    validate_get_users
    I=1

    ALL_ENTRIES=$($CURL -s $ADDR/get_params.cgi'?user='$USERNAME'&pwd='$PASSWORD | $EGREP 'user\S_')
    REGEX=".+'(.*)'.+"
    REGEX_ROLE=".+=(.*);"
    REGEX_IS_USER="_name"
    for e in $ALL_ENTRIES; do
        if [[ "$e" != "var" ]]; then
            if [[ $e =~ $REGEX ]]; then
                TOKEN=${BASH_REMATCH[1]}
                if [[ "$e" =~ $REGEX_IS_USER ]];then
                    SET_USER_PARAM="$SET_USER_PARAM"'user'$I"=$TOKEN"'&'
                else
                    SET_USER_PARAM="$SET_USER_PARAM"'pwd'$I"=$TOKEN"'&'
                fi
            elif [[ $e =~ $REGEX_ROLE ]]; then
                TOKEN=${BASH_REMATCH[1]}
                SET_USER_PARAM="$SET_USER_PARAM"'pri'$I"=$TOKEN"'&'
                I=$(expr $I + 1)
                if [[ $I == 8 ]]; then
                    break
                fi
            fi
        fi
    done
}


