#!/usr/bin/env bash
ROOT_DIR="${HOME}/heop_devel_kit"
DEFAULT_VOLUME_PATH="${ROOT_DIR}/volume"
TARGET_IMAGE=""
WITH_GPU="no"
NOTTY="no"

#merge
MERAGE_SOURCE_IMAGE=""
MERAGE_PLUGIN_IMAGE=""

function err() {
	echo -e "\033[31m!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\033[0m"
	echo -e "\033[31mERROR:$*\033[0m"
	echo -e "\033[31m!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\033[0m"
	exit 1
}

function info() {
	echo -e "\033[33m============================================\033[0m"
	echo -e "\033[33mINFO:$*\033[0m"
	echo -e "\033[33m============================================\033[0m"
}


function usage ()
{
	echo "
###########################################
#  Welcome to use $0  # v1.3.0
###########################################
    Usage :  $0 [-B] [-r] [-g] [-i image:tag] [-h] [-p] [-m] [-s source image] [-a plugin image]

    Options:
    -r|run         Run heop devel kit
    -n|notty       Run heop devel kit without pty(for ci auto test)
    -g|gpu         with gpu. use with -r
    -m|merge       merge heop_devel_kit with plugin image. See example below.

    -h|help        Display this message

    example:
    1. run with gpu enable
    $0 -r -g

    2. run without gpu enable
    $0 -r

    3. run with specify tag and enable gpu
    $0 -r -g -i <image_name:tag>

    4. run with specify tag and disable gpu
    $0 -r -i <image_name:tag>

    5. merge heop_devel_kit with plugin image
    $0 -m -s <source image> -a <plugin image>
"
}

function check() {
	local cmd
	local cmdlist="docker"
	
	for cmd in ${cmdlist}; do
		which ${cmd} > /dev/null 2>&1
		if [[ "$?" != "0" ]]; then
			err "missing command ${cmd}"
		fi
	done

	if [[ ! -d ${ROOT_DIR} ]]; then
		info "Can't find ROOT_DIR. Create ROOT_DIR in ${ROOT_DIR}"
		mkdir ${ROOT_DIR} || err "Create ROOT_DIR failed"
	fi
	
}

# Old version
# function load_config() {
# 	local vol_path
# 	if [[ ! -e ${ROOT_DIR}/.config ]]; then
# 		read -p "Please enter your volume path(user data path)[enter means uses default value ${DEFAULT_VOLUME_PATH}]:" vol_path
# 		case ${vol_path} in
# 			"")
# 				VOLUME_PATH=${DEFAULT_VOLUME_PATH};;
# 			*)
# 				VOLUME_PATH=${vol_path};;
# 		esac
# 		echo "VOLUME_PATH = ${VOLUME_PATH}"
# 		mkdir ${VOLUME_PATH} || err "create volume failed in ${VOLUME_PATH}"
# 		echo "VOLUME_PATH=${VOLUME_PATH}" > ${ROOT_DIR}/.config
# 	else
# 		echo "found config file and load ...."
# 		source ${ROOT_DIR}/.config
# 		echo "VOLUME_PATH=${VOLUME_PATH}"
# 	fi
# 	chmod 777 ${VOLUME_PATH}
# }

# New version
function load_config() {
    # 1. Если конфиг есть, спрашиваем, оставить ли старый путь
    if [[ -e ${ROOT_DIR}/.config ]]; then
        source ${ROOT_DIR}/.config
        echo -e "\033[33mFound existing VOLUME_PATH: ${VOLUME_PATH}\033[0m"
        read -p "Do you want to use this path? [Y/n]: " choice
        
        # Если пользователь ввел 'n' или 'N', сбрасываем переменную
        if [[ "$choice" == "n" || "$choice" == "N" ]]; then
            unset VOLUME_PATH
        fi
    fi

    # 2. Если пути нет (первый запуск ИЛИ пользователь отказался от старого)
    if [[ -z "${VOLUME_PATH}" ]]; then
        read -p "Please enter your volume path (user data path) [default ${DEFAULT_VOLUME_PATH}]: " vol_path
        # Используем введенный путь или дефолтный, если нажали Enter
        VOLUME_PATH=${vol_path:-$DEFAULT_VOLUME_PATH}
        
        # Сохраняем в конфиг (перезаписываем)
        echo "VOLUME_PATH=${VOLUME_PATH}" > ${ROOT_DIR}/.config
        mkdir -p "${VOLUME_PATH}" || err "Create volume failed in ${VOLUME_PATH}"
    fi

    echo "Using VOLUME_PATH: ${VOLUME_PATH}"
    chmod 777 "${VOLUME_PATH}"
}

# Old version
# function select_image(){
#	 images=`docker images| awk 'BEGIN{i=1} {if (NR>1) printf("[%4d] %s\n",i++,$0)}'`
#	 info "select heop_devel_kit image to run:\n$images"
#	 read -p "enter your choose:" index
#	 TARGET_IMAGE=`echo "$images" | awk -v idx="$index" 'NR==idx {printf("%s\n",$5)}'`
# }

# New version 
function select_image() {
    # 1. Получаем список образов в чистом виде: "repository:tag"
    local raw_images=$(docker images --format "{{.Repository}}:{{.Tag}}")
    
    # 2. Формируем нумерованный список для отображения пользователю
    local display_list=$(echo "$raw_images" | awk '{print "["NR"]", $1}')
    
    info "select heop_devel_kit image to run:\n$display_list"
    read -p "enter your choose: " index
    
    # 3. Проверяем ввод
    if [[ "$index" =~ ^[0-9]+$ ]]; then
        # Выбираем строку по номеру и берем только имя образа
        TARGET_IMAGE=$(echo "$raw_images" | sed -n "${index}p")
    else
        TARGET_IMAGE="$index"
    fi

    # Если выбор оказался пустым (неверный индекс)
    if [[ -z "$TARGET_IMAGE" ]]; then
        err "Invalid selection. Please try again."
    fi
}

function run_devel_kit() {
	local with_term="-it"
	if [[ $TARGET_IMAGE == "" ]];then
		select_image
	fi

	if [[ $NOTTY == "yes" ]];then
		with_term=""
	fi

	info "setup heop_devel_kit. IMAGE ID:\n$TARGET_IMAGE"
	if [ $(command -v nvidia-container-runtime) ]; then
		docker run --runtime nvidia --rm $with_term -u 0:0 -v /etc/localtime:/etc/localtime:ro --mount type=bind,src="${VOLUME_PATH}",dst="/heop/workspace" --cap-add=ALL $TARGET_IMAGE $@
	elif [ $(command -v nvidia-docker) ];then
		if [[ $WITH_GPU == "yes" ]]; then
			NV_GPU=0 nvidia-docker run  --rm $with_term -u 0:0 -v /etc/localtime:/etc/localtime:ro --mount type=bind,src="${VOLUME_PATH}",dst="/heop/workspace" --cap-add=ALL $TARGET_IMAGE $@
		else
			docker run --rm $with_term -u 0:0 -v /etc/localtime:/etc/localtime:ro --mount type=bind,src="${VOLUME_PATH}",dst="/heop/workspace" --cap-add=ALL $TARGET_IMAGE $@
		fi
	else
		if [[ $WITH_GPU == "yes" ]]; then
			err "Run image with gpu failed. Can't find nvidia-container or nvidia-docker package!"
		else
			docker run --rm $with_term -u 0:0 -v /etc/localtime:/etc/localtime:ro --mount type=bind,src="${VOLUME_PATH}",dst="/heop/workspace" --cap-add=ALL $TARGET_IMAGE $@
		fi
	fi
}


function merge_plugin() {
	#$1 basic image
	#$2 plugin image
	local URL=${1%:*}
	local TAG=${1#*:}_${2#*:}
	local workdir
	workdir=`mktemp -d /tmp/heop_devel_kit_merge.XXXX`
	cat << EOF > $workdir/dockerfile
#dockerfile auto generated by heop_devel_deploy.sh
FROM $2 AS PLUGIN
FROM $1 AS BASIC
COPY --from=PLUGIN /plugin/ /

EOF
	docker build -t $URL:$TAG $workdir  && info "merge plugin successfully" && info "use \"docker images\" to list. New image is $URL:$TAG"
	rm $workdir -rf
}


################ script start ##################

if [[ $# == 0 ]]; then
	usage
	exit 0;
fi

check


#-----------------------------------------------------------------------
#  Handle command line arguments
#-----------------------------------------------------------------------
while getopts ":hi:rgms:a:" opt
do
  case $opt in

	h|help     )
		usage
		exit 0
		;;
	i|image )
		TARGET_IMAGE=$OPTARG
		;;
	r|run  )
		image_opt="run"
		;;
	g|gpu  )
		WITH_GPU="yes"
		;;
	m|merge  )
		image_opt="merge"
		;;
	s)
		MERAGE_SOURCE_IMAGE=$OPTARG
		;;
	a)
		MERAGE_PLUGIN_IMAGE=$OPTARG
		;;
	* )  err "\n  Option does not exist : $OPTARG\n"
		  usage; exit 1   ;;
  esac    # --- end of case ---
done

shift $(($OPTIND-1))


case $image_opt in
	"run" )
		load_config
		run_devel_kit $@
		;;
	"merge" )
		if [[ -z "$MERAGE_PLUGIN_IMAGE" ]] || [[ -z "$MERAGE_SOURCE_IMAGE" ]];then
			err "please specify source image and plugin image"
		else
			merge_plugin $MERAGE_SOURCE_IMAGE $MERAGE_PLUGIN_IMAGE
		fi
		;;
esac

