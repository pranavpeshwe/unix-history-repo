#!/bin/sh
#-
# Copyright (c) 2014 The FreeBSD Foundation
# All rights reserved.
#
# This software was developed by Glen Barber under sponsorship
# from the FreeBSD Foundation.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
# mk-vmimage.sh: Create virtual machine disk images in various formats.
#
# $FreeBSD$
#

usage() {
	echo "${0} usage:"
	echo "${@}"
	return 1
}

main() {
	local arg
	while getopts "C:c:d:f:i:o:s:S:" arg; do
		case "${arg}" in
			C)
				VMBUILDCONF="${OPTARG}"
				;;
			c)
				VMCONFIG="${OPTARG}"
				;;
			d)
				DESTDIR="${OPTARG}"
				;;
			f)
				VMFORMAT="${OPTARG}"
				;;
			i)
				VMBASE="${VMBASE}"
				;;
			o)
				VMIMAGE="${OPTARG}"
				;;
			s)
				VMSIZE="${OPTARG}"
				;;
			S)
				WORLDDIR="${OPTARG}"
				;;
			*)
				;;
		esac
	done
	shift $(( ${OPTIND} - 1))

	if [ -z "${VMBASE}" -o \
		-z "${WORLDDIR}" -o \
		-z "${DESTDIR}" -o \
		-z "${VMSIZE}" -o \
		-z "${VMIMAGE}" -o \
		-z "${VMCONFIG}" ];
	then
		usage
	fi

	if [ -z "${VMBUILDCONF}" ] || [ ! -e "${VMBUILDCONF}" ]; then
		echo "Must provide the path to vmimage.subr."
		return 1
	fi

	. "${VMBUILDCONF}"

	if [ ! -z "${VMCONFIG}" ] && [ -e "${VMCONFIG}" ]; then
		. "${VMCONFIG}"
	fi

	vm_install_base
	vm_extra_install_base
	vm_extra_install_packages
	vm_extra_install_ports
	vm_extra_enable_services
	vm_extra_pre_umount
	vm_create_disk || return 0
	vm_extra_create_disk

	return 0
}

main "$@"
