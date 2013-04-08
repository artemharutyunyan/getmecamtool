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

validate_new_settings()
{
    if [[ -z $SETTINGS_FILE ]] || [[ ! -f $SETTINGS_FILE ]]
    then
       die "Could not locate settings file ($SETTINGS_FILE). Aborting validate_new_settings()"
    fi
    
    CONFEXTRACT=$(which confextract)
    if [[ -z $CONFEXTRACT ]]
    then
        die "Could not find confextract utility. Aborating validate_new_settings()"
    fi

    CONFPACK=$(which confpack)
    if [[ -z $CONFPACK ]]
    then
        die "Could not find confpack urility. Aboring validate_new_settings()"
    fi

    if [[ -z $EXT_SETTINGS_FILE ]] || [[ -z $NEW_SETTINGS_FILE ]]
    then
        die "settings file variables not defined. Aborting validate_new_settings()"
    fi
}

run_new_settings() 
{
    validate_new_settings
    $CONFEXTRACT -x $SETTINGS_FILE -o $EXT_SETTINGS_FILE
    $CONFPACK -f $SETTINGS_FILE -o $NEW_SETTINGS_FILE -s $EXT_SETTINGS_FILE
    $CONFEXTRACT -c $NEW_SETTINGS_FILE
    if [[ $? != 0 ]]
    then
        die "Could not verify new settings file ($NEW_SETTINGS_FILE). Aborting run_new_settings()"
    fi
}


