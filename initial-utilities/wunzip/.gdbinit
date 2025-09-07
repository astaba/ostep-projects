# ostep-projects/initial-utilities/wunzip/.gdbinit
# Create on: Sun Sep  7 19:32:14 +01 2025

# run this script with:
# prompt> gdb -q

# layout split
tui enable

file ./wunzip

break main

define skipops
	set $ino = $arg0
	while $ino > 0
		next
		set $ino = $ino - 1
	end
end

run ./tests/1.in
