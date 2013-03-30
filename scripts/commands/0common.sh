#!/bin/bash 

validate_admin()
{
    if [[ -z $USERNAME ]] || [[ -z $PASSWORD ]] || [[ -z $ADDR ]]
    then
        die "Operation xsxorequires username, password and address arguments. See $0 -h for details."
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


