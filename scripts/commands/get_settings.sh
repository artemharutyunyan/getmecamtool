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

validate_get_settings()
{
    validate_admin
    validate_curl
}

run_get_settings()
{
    validate_get_settings
    echo Trying to fetch settings from $ADDR
    $CURL $ADDR/backup_params.cgi'?user='$USERNAME'&pwd='$PASSWORD -o $SETTINGS_FILE >/dev/null 2>&1
}


