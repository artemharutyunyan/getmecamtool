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
export CMD_LIST=("${CMD_LIST[@]}" inject_proxy)

validate_inject_proxy()
{
    [[ -z $NEW_PORT ]] && die "Please specify the new port. See -h for details"

    validate_inject_exec
    get_network_params

    [[ ! -z $IP ]] || \ 
    [[ ! -z $MASK ]] || \
    [[ ! -z $GW ]] || \ 
    [[ ! -z $DNS ]] || \ 
    [[ ! -z $PORT ]] || \
        die "Network setting missing. Could not inject prox. "
}

run_inject_proxy()
{
    validate_inject_proxy
    
    echo "Trying to change port from $PORT to $NEW_PORT"
    # Change port number 
    CODE=$($CURL -s -o /dev/null -w "%{http_code}" \
        $ADDR/set_network.cgi'?user='$USERNAME'&pwd='$PASSWORD'&ip='$IP'&mask='$MASK'&dns='$DNS'&gateway='$GW'&port='$NEW_PORT
    )
        
    [[ $CODE == 200 ]] || die "Could not change port settings"
    green "Successfully changed port setting"

    [[ $PORT != $NEW_PORT  ]] || die "New port value can not be the same as current port value ($PORT)"

    ARG="$PORT $NEW_PORT"
    echo $ARGS 
    
    run_inject_exec
}


