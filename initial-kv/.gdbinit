# ostep-projects/initial-kv/.gdbinit
# Create on: Wed Sep 10 20:22:48 +01 2025

# run this script with:
# prompt> gdb -q

# layout split
tui enable

file ./kv

break main

define skipops
	set $ino = $arg0
	while $ino > 0
		next
		set $ino = $ino - 1
	end
end

# run "p,23,alpha" "p,87,bravo" "p,45,charlie" "p,89,delta" "p,60,echo"
run "p,49,paksylon"
# run "a"
# run "c"
run "d,39"
