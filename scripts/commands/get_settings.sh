# Add command to the array
export CMD_LIST=("${CMD_LIST[@]}" get_settings)

run_get_settings()
{
    echo "It works"
}

validate_get_settings()
{
if [[ -z $USERNAME ]] || [[ -z $PASSWORD ]] || [[ -z $ADDR ]]
then
     usage
     exit 1
fi    
}
