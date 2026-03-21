#!/bin/sh -e
##############################################################################
#
# FILE NAME  : eap-create.sh
#
# DESCRIPTION: Application package creation script to be run on the HOST
#
# ----------------------------------------------------------------------------
# Copyright (C) 2014, Axis Communications AB, LUND, SWEDEN
##############################################################################

#
# Note here are all internal functions
#
croak() {
	\printf "\n%s" "$*" >&2
}

###############################################################################
help() {
	local me
	me=${0##*/}
	croak "Use $me to create an Embedded Axis Package"
	croak "Usage:   $me <AXIS_BUILDTYPE>"
	croak "Example: $me mipsisa32r2el | armv6 | armv7 | armv7hf"
	croak ""
}

############################################################
cryptSymmetricPwd() {
	local pwd=$(openssl rand 45 -base64)
	echo $pwd
}

############################################################
cryptGetPwdFile() {
	local pwdin=$1

	local toolpath=$AXIS_TOP_DIR/tools/emb_app_sdk/scripts/
	local pubkeyfile=$toolpath/pub_rsa.pem
	local tmppwdfile=tmppwdfile.txt
	local encpwdfile=encpwd

	echo "$pwdin" > $tmppwdfile
	openssl rsautl -encrypt -inkey "$pubkeyfile" -pubin -in "$tmppwdfile" -out "$encpwdfile"
	rm $tmppwdfile

	echo "$encpwdfile"
}

############################################################
cryptEncryptScript() {
	local pwdin=$1
	local filein=$2
	local fileout=$3
	openssl enc -in "$filein" -k "$pwdin" -aes-256-cbc -nosalt -md md5 -out "$fileout"
}

############################################################
luaValidateFiles() {
	local invalid= ok=
	local match="${APPNAME%.xml}, $APPID, $APPMAJORVERSION, $APPMINORVERSION"

	\printf "Checking lua license setup... "

	for file in $LUAFILES $LUAFILESENCRYPTED; do
		local got=$(\sed -n "s/.*license\.Setup(\([^)]*\).*/\1/p" $file)
		if [ "$got" ]; then
			local stripped=$(echo "$got" | \sed -e "s/ *, */, /g" -e "s/[\"']//g")
			[ "$stripped" = "$match" ] || invalid="$file $invalid"
		fi
	done

	if [ "$invalid" ]; then
		\printf "invalid in: $invalid"
		\printf ""
		\printf "Please correct and try again."
		exit 1
	fi
	\printf "ok\n"
}

############################################################
luaSetLuaPgkFiles() {
	local skipped=

	TMPLUAFILESENCRYPTED=
	TMPSYMPWDFILEENCRYPTED=
	if [ "$LUAFILESENCRYPTED" ]; then
		local sympwd=$(cryptSymmetricPwd)
		\printf "Encryption of: '$LUAFILESENCRYPTED'... "
		for file in $LUAFILESENCRYPTED; do
			if [ -e $file ]; then
				\mv $file $file.orig
				cryptEncryptScript $sympwd $file.orig $file
				TMPLUAFILESENCRYPTED="$file $TMPLUAFILESENCRYPTED"
			else
				skipped="$skipped $file"
			fi
		done
		TMPSYMPWDFILEENCRYPTED=$(cryptGetPwdFile "$sympwd")
		if [ "$skipped" ]; then
			\printf "$skipped not found\n"
		else
			\printf "ok\n"
		fi
	fi

	#LUASTANDALONE applications have to have the is_running file with
	#"Not Running" content in it.
	if [ "$LUASTANDALONE" = yes ]; then
		echo "Not Running" > is_running
		LUASTANDALONEFILE=is_running
	fi
	LUAPKGFILES="$LUAFILES $TMPSYMPWDFILEENCRYPTED $TMPLUAFILESENCRYPTED $LUASTANDALONEFILE"
}

############################################################
luaCleanupGenFiles() {
	[ -z "$TMPSYMPWDFILEENCRYPTED" ] || \rm -f $TMPSYMPWDFILEENCRYPTED
	if [ "$LUAFILESENCRYPTED" ]; then
		for file in $LUAFILESENCRYPTED; do
			if [ -e $file.orig ]; then
				\mv $file.orig $file
			fi
		done
	fi
}

############################################################
doMakeTheTar() {
	local tarb

	# Create the name of the file.
	tarb=$(echo "$PACKAGENAME" | \sed 's/ /_/g')
	if [ "$APPMAJORVERSION" ] && [ "$APPMINORVERSION" ]; then
		tarb=${tarb}_${APPMAJORVERSION}_$APPMINORVERSION
		[ -z "$APPMICROVERSION" ] || tarb=${tarb}-$APPMICROVERSION
	fi

	if [ -z $APPTYPE ]; then
		tarb=$tarb.eap
	else
		tarb=${tarb}_$APPTYPE.eap
	fi

	LUAPKGFILES=
	[ "$LUAFILES" ] || [ "$LUAFILESENCRYPTED" ] && luaSetLuaPgkFiles

	\printf "Creating Package: '$tarb'... "
	[ ! -r $tarb ] || \mv -f $tarb $tarb.old

	\tar czf $tarb --exclude="*~" --exclude="CVS" --format=gnu $APPNAME $ADPPACKCFG $ADPPACKPARAMCFG \
		$POSTINSTALLSCRIPT $OTHERFILES $HTTPD_CONF_LOCAL_FILES \
		$HTTPD_MIME_LOCAL_FILES $HTMLDIR $EVENT_DECLS_DIR $LIBDIR \
		$LUAPKGFILES $HTTPCGIPATHS || {
			echo failed
			croak "Error: Could not create $tarb (FAILED)"
			exit 1
	}

	[ "$LUAFILES" ] || [ "$LUAFILESENCRYPTED" ] && luaCleanupGenFiles

	echo ok
}

# get_input_for_param()
#  * Read user-input for the specified parameter,
#    display default/previous value, and show help message.
#    If the user input does not validate, show error message
#    and loop until input is correct.
#
#  * Parameters
#    $1: Parameter name
#    $2: Help message to display

get_input_for_param() {
	local _param=$1 _msg=$2 _input _default

    eval _default=\$$_param

	\printf "$_msg" "$_default"

	read _input
	if [ -z "$_input" ]; then
		eval $_param=\$_default
	else
		eval $_param=\$_input
	fi

	while :; do
		if ! check_restriction_for_parameter $_param; then
			eval _default=\$$_param
			\printf  "$_msg" "$_default"
			read _input
			if [ -z "$_input" ]; then
				eval $_param=\$_default
			else
				eval $_param=\$_input
			fi
		else
			break
		fi
	done
}


# set_default_values()
#  * Set default value for all parameters
#
set_default_values() {
	local _p _ret _default _content F= IFS='
'
	for _p in $(list_package_conf_params); do
		get_default_value_for_param $_p _default
		eval _content=\$$_p
		[ "$_content" ] || eval $_p=\$_default
	done

	if [ -z "$APPNAME" ]; then
		# If we find exactly one executable or xml file in the
		# directory, suggest that file as default. Otherwise, empty
		# default.
		for F in *; do
			if [ ! -d $F ]; then
				if { [ "$APPTYPE" = lua ] && [ ${F%.xml} != $F ]; } ||
				   { [ "$APPTYPE" != lua ] && [ -x $F ]; }; then
					if [ "$APPNAME" ]; then
						# Allow only one match
						APPNAME=
						break
					fi
					APPNAME=$F
				fi
			fi
		done
	fi

	if [ "$APPTYPE" = lua ] && [ -z "$LUAFILES" ] && [ -z "$LUAFILESENCRYPTED" ]; then
		# Find lua files and use them as default
		for F in *; do
			if [ ! -d $F ] && [ ${F%.lua} != $F ]; then
				LUAFILES="$LUAFILES $F"
			fi
		done
		LUAFILES=${LUAFILES# }
	fi

	[ "$PACKAGENAME" ] || PACKAGENAME=$APPNAME
	[ "$APPTYPE" != lua ] || PACKAGENAME=${PACKAGENAME%.*}

	if [ -z "$SETTINGSPAGEFILE" ]; then
		if [ -r html/settings.html ]; then
			SETTINGSPAGEFILE=settings.html
			SETTINGSPAGETEXT="Advanced settings"
		fi
	fi
}


# genPackCfg()
#  * Generate the package.conf file based on user input
#
genPackCfg() {
	local _p _helptext _do_display _is_lua IFS='
'
	for _p in $(list_package_conf_params); do
		_do_display=yes
		_is_lua=no

		case $_p in
			APPGRP|APPUSR)
				[ "$APPTYPE" != lua ] || _do_display=no
				;;
			LUAFILES|LUAFILESENCRYPTED|LUASTANDALONE)
				[ "$APPTYPE" = lua ] || _do_display=no
				;;
			LICENSE_CHECK_ARGS)
				[ "$LICENSEPAGE" = custom ] || _do_display=no
				;;
			*)
				;;
		esac

		if [ $_do_display = yes ]; then
			if [ "$APPTYPE" = lua ]; then
				get_lua_helptext_for_param $_p _helptext
			else
				get_helptext_for_param $_p _helptext
			fi
			_helptext="$_p: $_helptext [%s] "
			get_input_for_param $_p "$_helptext"
		fi
	done

	generate_adppackcfg
	\printf "package.conf created.\n"
}

############################################################
checkNum() {
	[ $# -eq 1 ] || return 1
	case $1 in
		*[![:digit:]]*)
			return 1
			;;
	esac
}

############################################################
checkAlphaNumSpace() {
	[ $# -eq 1 ] || return 1
	case $1 in
		*[![:alnum:][:blank:]]*)
			return 1
			;;
	esac
}

############################################################
checkAlphaNumAscii() {
	[ $# -eq 1 ] || return 1
	case $1 in
		*[![:alnum:]]*)
			return 1
			;;
	esac
}

############################################################
checkVersionNum() {
	local _tmp

	_tmp=$(echo "$1" | \sed -r -e 's/([^0-9.]|[.]{2,}|^\.|\.$)//g')
	[ "$_tmp" = "$1" ]
}

############################################################
# Makes sure that the string provided in $1 is made up only of characters
# the matches the pattern provided in $2. An empty $1 is always OK.
checkPattern() {
	local _tmp

	[ $# -eq 2 ] || return 1
	[ "$2" ] || return 1

	_tmp=$(echo "$1" | \sed -e "s/$2//g" 2>/dev/null)
	[ -z "$_tmp" ]
}

############################################################

# Checking inputs
# 1. Variables should in general only contain valid alphanumeric
#    characters and digits, no punctuation, no special characters.
# 2. APPNAME should only include alphanum for binary applications.
#    For LUA-applications, dots need to be allowed as well since
#    the file name extension is ".xml".
# 3. PACKAGENAME, which is a nice name used in the web page,
#    may contain spaces and dashs.
# 4. MENUNAME, which is a nice name used in the web menu,
#    may contain spaces and dashs.

checkallerrors() {
	local _p= _msg= _error=0 IFS='
'
	for _p in $(list_package_conf_params); do
		# Check restriction for each parameter
		if ! _msg=$(check_restriction_for_parameter $_p); then
			croak "[Error] $_msg"
			_error=1
			break
		fi
	done

	# this returns a string
	echo $_error
}

#######################################################
createPackageConf() {
	# Set default values for unset parameters
	set_default_values

	\printf "Validating Package config...\n"

	while [ "$(checkallerrors)" -ne 0 ]; do
		\printf "\n"
		croak "We need to fix $ADPPACKCFG Please answer the following questions:"
		\printf "\n\n"
		genPackCfg
	done

	if [ "$APPTYPE" != lua ] && [ ! -r $ADPPACKPARAMCFG ]; then
		\touch $ADPPACKPARAMCFG || exit 1
		croak "Info: Created an empty $ADPPACKPARAMCFG"
	fi

	\printf "Saving Package config: '%s'..." "$ADPPACKCFG"
	[ -r $ADPPACKCFG ] && \mv -f $ADPPACKCFG $ADPPACKCFG.orig

	generate_adppackcfg
	\printf "ok\n"
}


# generate_adppackcfg()
#  * Generate and write the package.conf file 'package.conf'
#
generate_adppackcfg() {
	local _p _content _do_write
	local IFS='
'
	\rm -f $ADPPACKCFG || :
	for _p in $(list_package_conf_params); do
		eval _content=\$$_p
		_do_write=no

		case $_p in
			LUAFILES|LUAFILESENCRYPTED)
				[ $APPTYPE != lua ] || _do_write=yes
				;;
			APPUSR|APPGRP|APPOPTS)
				[ $APPTYPE = lua ] || _do_write=yes
				;;
			LICENSE_CHECK_ARGS)
				[ $LICENSEPAGE = custom ] || _do_write=no
				;;
			LUASTANDALONE)
				if [ "$APPTYPE" = lua ]; then
					[ -z "$LUASTANDALONE" ] || _do_write=yes
				fi
				;;
			*)
				_do_write=yes
				;;
		esac

		if [ $_do_write = yes ]; then
			case $_p in
				VENDORHOMEPAGELINK)
					\printf "%s=\'%s\'\n" $_p "$_content" >> $ADPPACKCFG
					;;
				*)
					\printf "%s=\"%s\"\n" $_p "$_content" >> $ADPPACKCFG
					;;
			esac
		fi
	done
}

# list_package_conf_all()
# * list PARAMETER RESTRICTION_TYPE DEFAULTVALUE for each PARAMETER
#
list_package_conf_all(){
	local _exclude1=_DESCRIPTIVE_HELPTEXT
	local _exclude2=_DESCRIPTIVE_ERRORMSG
	local _exclude3=_DESCRIPTIVE_LUA_HELPTEXT

	\cat $DEFCFG | \sed '/#/d' | \sed '/./!d' | \grep -v $_exclude1 | \grep -v $_exclude2 | \grep -v $_exclude3
}

# list_package_conf_helptext()
# * list all help text
#
list_package_conf_helptext(){
	local helptext="_DESCRIPTIVE_HELPTEXT="

	\cat $DEFCFG | \sed '/#/d' | \sed '/./!d' | \grep $helptext
}

# list_package_conf_lua_helptext()
# * list all Lua help text
#
list_package_conf_lua_helptext(){
	local helptext="_DESCRIPTIVE_LUA_HELPTEXT="

	\cat $DEFCFG | \sed '/#/d' | \sed '/./!d' | \grep $helptext
}

# list_package_conf_errormsg()
# * list all error messages
#
list_package_conf_errormsg(){
	local errormsg="_DESCRIPTIVE_ERRORMSG="

	\cat $DEFCFG | \sed '/#/d' | \sed '/./!d' | \grep $errormsg
}

# list_package_conf_params_and_restrictions()
#  * Generate list of package.conf parameters
#    including the parameter restriction to stdout
#
list_package_conf_params_and_restrictions(){
	local _line _param _restriction IFS='
'
	for _line in $(list_package_conf_all); do
		_param=${_line%% *}
		_restriction=${_line#* }
		_restriction=${_restriction%% *}
		\echo $_param $_restriction
	done
}

# gen_package_conf_params_and_restrictions()
#  * Write generated list of parameters and
#    restrictions to file
#
gen_package_conf_params_and_restrictions(){
	list_package_conf_params_and_restrictions >$DEFLIST
}

# list_package_conf_params()
#  * Generate list of package.conf variables without
#    the content restriction information to stdout.
#
list_package_conf_params(){
	local _line _var IFS='
'
	for _line in $(list_package_conf_params_and_restrictions) ; do
		_var=${_line%% *}
		\echo $_var
	done
}

# gen_package_conf_params()
# * Write generated list of parameters only to file
#
gen_package_conf_params(){
	list_package_conf_params >$DEFPARM
}

# check_restriction_for_parameter()
#   * Check the RESTRICTION type for the specified parameter
#   * Parameters:
#      $1: Parameter name to check
#   * Return:
#      0 : if parameter content is valid
#      1 : if parameter content is invalid according to the restriction
#
check_restriction_for_parameter(){
	local _line _var _restr _varcontent _ret=1 _str _file _parameter=$1
	local _restriction_license="none axis custom"
	local _restriction_startmode="never once"
	local _restriction_standalone_mode="yes no"

	# Parameter type restrictions
	case $_parameter in
		APPTYPE)
			echo "checking APPTYPE '$APPTYPE'"
			;;
		APPNAME)
			if [ -z "$APPNAME" ]; then
				echo "* $_parameter cannot be empty"
				return 1
			else
			if [ ! -r "$APPNAME" ]; then
					if [ "$APPTYPE" = lua ]; then
						echo "* Could not find application XML configuration file '$APPNAME' in the current dir"
					else
						echo "* Could not find application binary file '$APPNAME' in the current dir"
					fi
					return 1
				fi
			fi
			;;
		PACKAGENAME)
			if [ -z "$PACKAGENAME" ]; then
				echo "* $_parameter cannot be empty"
				PACKAGENAME=$APPNAME
				[ "$APPTYPE" != lua ] || PACKAGENAME=${PACKAGENAME%.*}
				return 1
			fi
			;;
		OTHERFILES)
			[ "$OTHERFILES" ] || OTHERFILES=
			local IFS=" "
			for _file in $OTHERFILES; do
				[ -e $_file ] || {
					echo "* File not found in $_parameter: '$_file'"
					return 1
				}
			done
			;;
		LUAFILES)
			local IFS=" "
			for _file in $LUAFILES ; do
				[ -e $_file ] || {
					echo "* File not found in $_parameter: '$_file'"
					return 1
				}
			done
			;;
		LUAFILESENCRYPTED)
			local IFS=" "
			for _file in $LUAFILESENCRYPTED ; do
				[ -e $_file ] || {
					echo "* File not found in $_parameter: '$_file'"
					return 1
				}
			done
			;;
		APPOPTS)
			[ "$APPOPTS" ] || APPOPTS=
			;;
		SETTINGSPAGEFILE)
			[ "$SETTINGSPAGEFILE" ] || SETTINGSPAGEFILE=
			[ -r "html/settings.html" ] && [ ! "$SETTINGSPAGEFILE" ] && {
				echo "* html/settings.html found, but no SETTINGSPAGEFILE defined"
				return 1
			}
			;;
		SETTINGSPAGETEXT)
			[ "$SETTINGSPAGETEXT" ] || SETTINGSPAGETEXT=
			;;
		VENDORHOMEPAGELINK)
			[ "$VENDORHOMEPAGELINK" ] || VENDORHOMEPAGELINK=
			;;
		LUASTANDALONE)
			[ "$APPTYPE" = lua ] || return 0
			;;
		STARTMODE)
			if [ "$APPTYPE" = lua ]; then
				_restriction_startmode="$_restriction_startmode auto"
			else
				_restriction_startmode="$_restriction_startmode respawn"
			fi
			;;
		LICENSE_CHECK_ARGS)
			[ "$LICENSE_CHECK_ARGS" ] || return 0
			;;
		*)
			;;
	esac


	_line=$(list_package_conf_params_and_restrictions | \grep -w $_parameter)
	_var=${_line%% *}
	_restr=${_line##* }
	eval _varcontent=\$$_var

	if [ "$_parameter" != "$_var" ]; then
		croak "Wanted Parameter \"%s\" differs from listed param \"%s\"" $_parameter $_var
		croak "Content restriction missmatch"
		return 1
	fi

	# Validate the parameter content
	case $_restr in
		RESTRICTION_ALPHANUM)
			if ! checkAlphaNumAscii "$_varcontent"; then
				echo "* $_var must consist of only letters and numbers"
			else
				_ret=0
			fi
			;;

		RESTRICTION_ALPHANUM_SPACE)
			if ! checkAlphaNumSpace "$_varcontent"; then
				echo "* $_var must consist of only letters, numbers and space"
			else
				_ret=0
			fi
			;;

		RESTRICTION_ALPHANUM_DOT)
			case $_varcontent in
				''|*[![:alnum:].]*)
					echo "* $_var must consist of letters, numbers and dots"
					;;
				*)
					_ret=0
					;;
			esac
			;;

		RESTRICTION_ALPHANUM_DASH_UNDERSCORE)
			case $_varcontent in
				''|*[![:alnum:]_-]*)
					echo "* $_var must consist of only letters, numbers, dash and underscores"
					;;
				*)
					_ret=0
					;;
			esac
			;;

		RESTRICTION_VERSION)
			if ! checkVersionNum "$_varcontent"; then
				echo "* $_var must consist of only numbers"
			else
				_ret=0
			fi
			;;

		RESTRICTION_NUM)
			if ! checkNum "$_varcontent"; then
				echo "* $_var must consist of numbers"
			else
				_ret=0
			fi
			;;

		RESTRICTION_NAME)
			case $_varcontent in
				''|*[![:alnum:][:blank:]._-]*)
					echo "* $_var must consist of only letters, blanks, dashes, dots, underscores or numbers"
					;;
				*)
					_ret=0
					;;
			esac
			;;

		RESTRICTION_STRING_OR_EMPTY)
			if ! checkPattern "$_varcontent" "[[:alnum:][:blank:]-]"; then
				echo "* $_var must consist of only letters, numbers and dash OR be an empty string"
			else
				_ret=0
			fi
			;;

		RESTRICTION_BINARY)
			local name="$_varcontent"

			if [ "$APPTYPE" = lua ]; then
				name="${_varcontent%.xml}"
			fi
			case $name in
				''|*[![:alnum:]_]*)
					echo "* $_var must consist of only letters, numbers or underscores"
					;;
				*)
					_ret=0
					;;
			esac
			;;

		RESTRICTION_FILE_OR_EMPTY)
			local _f= _tmp_ret=0 IFS=" "

			if [ "$_varcontent" ]; then
				for _f in $_varcontent ; do
					case $_f in
						*[![:alnum:]._/-]*)
							echo "* $_var - invalid filename/pathname: \"$_f\""
							_tmp_ret=1
							;;
					esac
				done
				[ $_tmp_ret -ne 0 ] || _ret=0
			else
				_ret=0
			fi
			;;

		RESTRICTION_LICENCEPAGE)
			local IFS=" "

			for _str in $_restriction_license ; do
				if [ "$_varcontent" = $_str ]; then
				    _ret=0
				fi
			done

			if [ $_ret -ne 0 ]; then
				echo "* $_var must be set to one of the following: \"$_restriction_license\""
			fi
			;;
		RESTRICTION_LUASTANDALONE)
			local IFS=" "

			for _str in $_restriction_standalone_mode ; do
				if [ "$_varcontent" = $_str ]; then
				    _ret=0
				fi
			done

			if [ $_ret -ne 0 ]; then
				echo "* $_var must be set to one of the following: \"$_restriction_standalone_mode\""
			fi
			;;

		RESTRICTION_STARTMODE)
			local IFS=" "

			for _str in $_restriction_startmode ; do
				if [ "$_varcontent" = $_str ]; then
					_ret=0
				fi
			done

			if [ $_ret -ne 0 ]; then
				echo "* Invalid $_var. Valid values are: \"$_restriction_startmode\""
			fi
			;;

		RESTRICTION_NONE)
			_ret=0
			;;

		*)
			# Assume this is a regular expression, for which an
			# empty default value is allowed
			if checkPattern "$_varcontent" "$_restr"; then
				_ret=0
			else
				echo "* Invalid $_var=$_varcontent. Valid values are described by the following reg-exp $_restr"
			fi
			;;
	esac

	[ $_ret -eq 0 ] || _ret=1

	return $_ret
}

# get_restriction_for_param()
#  * Get restriction for parameter#
#  * Parameters:
#     $1 :  The wanted parameter
#     $2 :  The variable name to store the restriction expression in
#
get_restriction_for_param() {
	local _restr _parameter=$1

	if [ -z "$_parameter" ]; then
		croak "[Ooops!] missing argument to get_restriction_for_param()"
		return 1
	fi

	_restr=$(list_package_conf_params_and_restrictions | \grep -w $_parameter)
	eval $2=\$$_restr
}

# get_default_value_for_param()
#   * Find the default parameter value for
#     the specified parameter.
#
#   * Arguments
#     $1: Parameter
#     $2: variable name to store the default value.
#
get_default_value_for_param() {
	local _tmp _pline _param _restriction _defaultvalue
	local _parameter=$1

	if [ -z "$_parameter" ]; then
		croak "[Ooops!] missing argument \$1 to get_default_value_for_param()"
		return 1
	fi

	if [ -z "$2" ]; then
		croak "[Ooops!] missing argument \$2 to get_default_value_for_param()"
		return 1
	fi

	_pline=$(list_package_conf_all | \grep -w $_parameter)
	_param=${_pline%% *}
	_tmp=${_pline#* }
	_restriction=${_tmp%% *}
	_defaultvalue=${_tmp#* }

    if [ "$_parameter" != "$_param" ] ; then
		croak "[Ooops!] Wanted: $_parameter got: $_param"
	fi

	eval $_defaultvalue
	eval $2=\$DEFAULTVALUE
	unset DEFAULTVALUE
}

# exist_and_sourceable()
#   * Test if the file specified in $1 exist and is sourceable.
#   * exit status
#      0 - file is sourceable
#      1 - file is not sourceable
#   * Arguments:
#      $1: File to test
#
exist_and_sourceable() {
	local _file=$1 _ret=1

	if [ -z "$_file" ]; then
		croak "[Ooops!] missing argument to exist_and_sourceable()"
		return 1
	fi

    if [ ! -f "./$_file" ] ; then
		croak "No $_file found"
	    touch ./$_file
		croak "Created $_file using default values... ok"
		set_default_values
	fi

	if $(. "./$_file" 2>/dev/null) ; then
		_ret=0
	fi

	return $_ret
}

# get_helptext_for_param()
#  * Get helptext for the specified parameter
#    Help text will be stored in variable "$1_DESCRIPTIVE_HELPTEXT"
#    where $1 expands to the parameter name.
#    Or in $2 if $2 is specified.
#
#  * Arguments:
#    $1: Parameter name
#    $2: Variable name to store the help text in
#
get_helptext_for_param() {
    local helptxt=_DESCRIPTIVE_HELPTEXT
    local _parameter=$1 _retval=$2
    local _txt _str

	if [ -z "$_parameter" ]; then
		croak "[Ooops!] missing argument \$1 to get_helptext_for_param()"
		return 1
	fi

	if _txt=$(list_package_conf_helptext | \grep -w "$_parameter$helptxt") ; then
		\eval $_txt
		\eval _str=$_parameter$helptxt
		[ -z "$2" ] || \eval $_retval=\$$_str
	else
		[ -z "$2" ] || \eval $_retval=
	fi
}

# get_lua_helptext_for_param()
#  * Get lua specific helptext for the specified parameter
#    Help text will be stored in variable "$1_DESCRIPTIVE_LUA_HELPTEXT"
#    where $1 expands to the parameter name.
#    Or in $2 if $2 is specified.
#
#  * Arguments:
#    $1: Parameter name
#    $2: Variable name to store the help text in
#
get_lua_helptext_for_param() {
    local helptxt=_DESCRIPTIVE_LUA_HELPTEXT
    local _parameter=$1 _retval=$2
    local _txt _str

	if [ -z "$_parameter" ]; then
		croak "[Ooops!] missing argument \$1 to get_lua_helptext_for_param()"
		return 1
	fi

	if _txt=$(list_package_conf_lua_helptext | \grep -w "$_parameter$helptxt") ; then
		\eval $_txt
		\eval _str=$_parameter$helptxt
		[ -z "$2" ] || \eval $_retval=\$$_str
	else
		[ -z "$2" ] || \eval $_retval=
	fi
}
#
#  debug()
#
debug_print_config(){
	local _p _ret _content _help
	local IFS='
'
	for _p in $(list_package_conf_params) ; do
		eval _content=\$$_p
		#\printf "var:%-20s <--> \t content=%-20s" $_p $_content
		check_restriction_for_parameter $_p
		_ret=$?
		if [ $_ret -eq 0 ] ; then
		  true #  \printf "<PASSED>\n"
		else
		   true # \printf "<FAILED>\n"
		fi

		get_helptext_for_param $_p _help
		#echo "HELPTEXT@$_p : \"$_help\""
		\printf "%-20s: %-48s %-16s \n" $_p "\"$_help\"" "[$_content]"
	done
}

################################################################################
# End of all functions, here starts our business
################################################################################
me=${0##*/}
LANG=C

if [ -z "$AXIS_TOP_DIR" ]; then
	croak "AXIS_TOP_DIR is not set"
	croak "To fix this, please do the following:"
	croak "-------------------------------------------------------------------------"
	croak "cd ../.."
	croak ". init_env"
	croak  cd `command pwd`
	croak "-------------------------------------------------------------------------"
	exit 1
fi


if [ -d $AXIS_TOP_DIR/tools/scripts ]; then
	script_path=tools/scripts
elif [ -d $AXIS_TOP_DIR/tools/emb_app_sdk/scripts ]; then
	script_path=tools/emb_app_sdk/scripts
else
	printf "\n [Error] cannot find script-path - Check you SDK installation\n"
	exit 1
fi

DEFCFG=$AXIS_TOP_DIR/$script_path/package-conf-parameters.cfg
DEFLIST=$AXIS_TOP_DIR/$script_path/package-conf-parameters.full
DEFPARM=$AXIS_TOP_DIR/$script_path/package-conf-parameters.parm

case "$1" in
	-h)
		help
		exit 0
		;;
esac

#Check comannd line parameters
if [ -z "$1" ]; then
	help
	exit 100
fi

HTTPD_CONF_LOCAL="httpd.conf.local"
HTTPD_MIME_LOCAL="mime.types.local"
HTTPD_CONF_LOCAL_FILES=`echo $HTTPD_CONF_LOCAL.*`
HTTPD_MIME_LOCAL_FILES=`echo $HTTPD_MIME_LOCAL.*`
if [ "$HTTPD_CONF_LOCAL_FILES" = "$HTTPD_CONF_LOCAL.*" ] ; then
	HTTPD_CONF_LOCAL_FILES=
fi
if [ "$HTTPD_MIME_LOCAL_FILES" = "$HTTPD_MIME_LOCAL.*" ] ; then
	HTTPD_MIME_LOCAL_FILES=
fi

ADPPACKCFG=package.conf

if exist_and_sourceable $ADPPACKCFG ; then
   \printf "\nReading local %s... " $ADPPACKCFG
	#remove possible CRs (Windows style end of line)
	\sed -i -e 's/\r$//' $ADPPACKCFG
   . ./$ADPPACKCFG
   \printf "ok\n"
else
  croak "Found errors in $ADPPACKCFG"
  rm -f ./$ADPPACKCFG
fi

# This list of targets and corresponding processors needs to be
# maintained and updated when new processors are added!
case $1 in
    artpec-4 | artpec-5 | mipsisa32r2el | mipsisa32r2el-axis-linux-gnu)
	APPTYPE=mipsisa32r2el
	;;
    ambarella-a5s | armv6 | armv6-axis-linux-gnueabi)
	APPTYPE=armv6
	;;
    ambarella-s2 | armv7 | armv7-axis-linux-gnueabi)
	APPTYPE=armv7
	;;
    armv7hf | armv7-axis-linux-gnueabihf)
	APPTYPE=armv7hf
	;;
    lua)
	APPTYPE=lua
	;;
    *)
	\printf "Unknown target: '$1'!\n"
	help
	exit 100
	;;
esac

[ "$APPTYPE" = lua ] || ADPPACKPARAMCFG=param.conf
HTMLDIR=html
[ -d $HTMLDIR ] || HTMLDIR=

EVENT_DECLS_DIR=declarations
[ -d $EVENT_DECLS_DIR ] || EVENT_DECLS_DIR=

LIBDIR=lib
[ -d $LIBDIR ] || LIBDIR=

createPackageConf
[ "$LUAFILES" ] && [ "$LUAFILESENCRYPTED" ] && luaValidateFiles
doMakeTheTar

\printf "\nTo install and run the package, you can use a browser and surf to the following page,"
\printf "\n(replace axis_device_ip with the IP number of your Axis video product)"
\printf "\n\nhttp://axis_device_ip/devtools.shtml"
\printf "\n\nOr simply use the script and type"
\printf "\neap-install.sh install"
\printf "\n\n"

exit 0
