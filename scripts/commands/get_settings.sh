#!/bin/bash

# Add command to the array
export CMD_LIST=("${CMD_LIST[@]}" get_settings)

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


