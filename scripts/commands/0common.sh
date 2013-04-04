#!/bin/bash 

validate_admin()
{
    if [[ -z $ADDR ]]
    then
        die "Operation requires an address argument. See $0 -h for details."
    fi   
}

validate_curl()
{
    CURL=$(which curl)
    [[ -z $CURL ]] && die "curl not found in \$PATH"
}

validate_grep()
{
    GREP=$(which grep)
    [[ -z $GREP ]] && die "grep not found in \$PATH"
}

validate_egrep()
{
    EGREP=$(which egrep)
    [[ -z $EGREP ]] && die "egrep not found in \$PATH"
}

validate_lib()
{
    if [[ -z $SYS_FW_LIB ]] || [[ ! -d $SYS_FW_LIB ]]
    then
        die "System firmware library directory must be provided (and must exist $SYS_FW_LIB) as an argument to inject_exec command."
    fi
}

get_sys_version()
{
    validate_admin
    validate_curl 
    validate_grep 

    SYS_VERSION=$($CURL -s $ADDR/get_params.cgi'?user='$USERNAME'&pwd='$PASSWORD | $GREP sys_ver)
    [[ $? == 0 ]] || die "Error fetching parameters from $ADDR"
    REGEX=".+'(.+)'.+"
    if [[ "$SYS_VERSION" =~ $REGEX ]]; then
        SYSTEM_VERSION=${BASH_REMATCH[1]}
    else
        die "Could not extract system version from $SYS_VERSION"
    fi
}

get_webui_version()
{
    validate_admin
    validate_curl
    validate_grep 

    WEBUI_VERSION=$($CURL -s $ADDR/get_params.cgi'?user='$USERNAME'&pwd='$PASSWORD | $GREP app_ver)
    [[ $? == 0 ]] || die "Error fetching parameters from $ADDR"
    REGEX=".+'(.+)'.+"
    if [[ "$WEBUI_VERSION" =~ $REGEX ]]; then
        WEBUI_VERSION=${BASH_REMATCH[1]}
    else
        die "Could not extract system version from $SYS_VERSION"
    fi
}

get_network_params()
{
    validate_admin
    validate_curl
   
    ALL_ENTRIES=$($CURL -s $ADDR/get_params.cgi'?user='$USERNAME'&pwd='$PASSWORD) 
    REGEX_IP="ip='(.+)'"
    REGEX_MASK="mask='(.+)'"
    REGEX_GW="gateway='(.+)'"
    REGEX_DNS="dns='(.+)'"
    REGEX_PORT="^port=(.+);"

    for e in $ALL_ENTRIES; do
        if [[ $e =~ $REGEX_IP ]]; then
            IP=${BASH_REMATCH[1]}
        elif [[ $e =~ $REGEX_MASK ]]; then
            MASK=${BASH_REMATCH[1]}
        elif [[ $e =~ $REGEX_GW ]]; then
            GW=${BASH_REMATCH[1]}
        elif [[ $e =~ $REGEX_DNS ]]; then
            DNS=${BASH_REMATCH[1]}
        elif [[ $e =~ $REGEX_PORT ]]; then
            PORT=${BASH_REMATCH[1]}
       fi
    done
}

RED="\033[31m"
GREEN="\033[32m"
DEFAULT="\033[m\017"

green()
{
    echo -e ${GREEN}"$1"${DEFAULT}
}

red()
{
    echo -e ${RED}"$1"${DEFAULT} 1>&2
}

die()
{
  red "$1"
  exit 1
}


