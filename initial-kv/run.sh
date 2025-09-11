#!/bin/bash

case "${1}" in
	p6)
		./kv "p,23,alpha" "p,87,bravo" "p,45,charlie" "p,89,delta" "p,60,echo"
		;;
	# a)
	# 	./kv a
	# 	;;
	*)
		command echo "${0}: Unsupported entry" 1>&2
		exit 1
		;;
esac
