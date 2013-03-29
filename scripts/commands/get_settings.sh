#!/bin/bash

# Add command to the array
export CMD_LIST=("${CMD_LIST[@]}" get_settings)

validate_get_settings()
{
    if [[ -z $USERNAME ]] || [[ -z $PASSWORD ]] || [[ -z $ADDR ]]
    then
        die "get_settings commands requires username, password and address arguments. See $0 -h for details."
    fi   
    
    CURL=$(which curl)
    [[ -z $CURL ]] && die "curl not found in \$PATH"
}

run_get_settings()
{
    validate_get_settings
    echo Trying to fetch settings from $ADDR
    $FILTER $CURL $ADDR/backup_params.cgi'?user='$USERNAME'&pwd='$PASSWORD -o $SETTINGS_FILE >/dev/null 2>&1
}


