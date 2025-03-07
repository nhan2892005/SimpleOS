#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-only
#
# Generate a syscall table header.
#
# Each line of the syscall table should have the following format:
#
# NR ABI NAME [NATIVE] [COMPAT]
#
# NR       syscall number
# ABI      ABI name
# NAME     syscall name
# NATIVE   native entry point (optional)
# COMPAT   compat entry point (optional)

set -e

usage() {
	echo >&2 "usage: $0 INFILE OUTFILE" >&2
	echo >&2
	echo >&2 "  INFILE    input syscall table"
	echo >&2 "  OUTFILE   output header file"
	echo >&2
	exit 1
}


if [ $# -ne 2 ]; then
	usage
fi

infile="$1"
outfile="$2"

nxt=0

grep -E "^[0-9]+[[:space:]]+" "$infile" | sort -n | {

	while read nr name native ; do

#		while [ $nxt -lt $nr ]; do
#			echo "__SYSCALL($nxt, sys_ni_syscall)"
#			nxt=$((nxt + 1))
#		done

		if [ -n "$native" ]; then
			echo "__SYSCALL($nr, $native)"
#		else
#			echo "__SYSCALL($nr, sys_ni_syscall)"
		fi
		nxt=$((nr + 1))
	done
} > "$outfile"
