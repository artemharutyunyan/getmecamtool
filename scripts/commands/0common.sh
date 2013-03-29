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
    REGEX=".+'(.+)'.+"
    if [[ "$SYS_VERSION" =~ $REGEX ]]; then
        SYSTEM_VERSION=${BASH_REMATCH[1]}
    else
        die "Could not extract system version from $SYS_VERSION"
    fi
}


