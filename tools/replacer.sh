
# copy template to destination
cp $2 $3
# replace / with \/ in $1
REPLACED=$(echo $1 | sed 's/\//\\\//g')
# replace compiler info
UNAMEOUT="$(uname -s)"
case "${UNAMEOUT}" in
    Linux*)    
        sed -i 's/#COMPLILER_INFO#/'$REPLACED'/g' $3
        ;;
    Darwin*)    
        sed -i'.original' -e 's/#COMPLILER_INFO#/'$REPLACED'/g' $3
        ;;
esac
