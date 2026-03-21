#!/bin/sh -e
##############################################################################
#
# FILE NAME  : create-package.sh
#
# DESCRIPTION: Application package creation script
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

help() {
	local me
	me=${0##*/}
	croak "Use $me to create Embedded Axis Packages"
	croak "Usage:   $me [<AXIS_BUILDTYPES>]"
	croak "Example: $me"
	croak "Example: $me mipsisa32r2el | armv6 | armv7 | armv7hf"
	croak ""
}

#
################################################################################
# End of all functions, here starts our business
################################################################################

TARGET_LIST="mipsisa32r2el-axis-linux-gnu \
             armv6-axis-linux-gnueabi \
             armv7-axis-linux-gnueabi \
             armv7-axis-linux-gnueabihf"

case "$1" in
	-h)
		help
		exit 0
		;;
esac

# If argument is specified then check it against available processors.
# Otherwise loop over all possible processors and build for all.
if [ $# -ge 1 ]; then
	TARGETS=$(echo $1 | tr '[:upper:]' '[:lower:]')
else
	# loop over all possible targets
	TARGETS=
	for target in $TARGET_LIST ; do
		[ -d $AXIS_TOP_DIR/target/$target ] && TARGETS="$target $TARGETS"
	done
fi

# The chip names are allowed as targets for backward compatability.
for target in $TARGETS ; do
	case "$target" in
		artpec-4 | artpec-5 | mipsisa32r2el)
			TARGET=mipsisa32r2el-axis-linux-gnu
			;;
		ambarella-a5s | armv6)
			TARGET=armv6-axis-linux-gnueabi
			;;
		ambarella-s2 | armv7)
			TARGET=armv7-axis-linux-gnueabi
			;;
		armv7hf)
			TARGET=armv7-axis-linux-gnueabihf
			;;
		mipsisa32r2el-axis-linux-gnu | \
		armv6-axis-linux-gnueabi | \
		armv7-axis-linux-gnueabi | \
		armv7-axis-linux-gnueabihf)
			TARGET=$target
			;;
		*)
			\printf "Unknown target: '$target'!\n"
			help
			exit 1
			;;
	esac
	\printf "make $TARGET"
	\make $TARGET
	\printf "make"
	\make || {
		croak "make failed. Please fix above errors, before you can create a package"
		exit 1
	}
	\command -v eap-create.sh > /dev/null 2>&1 && \eap-create.sh $TARGET || `dirname $0`/eap-create.sh $TARGET
done

exit 0
