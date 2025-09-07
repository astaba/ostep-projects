# ostep-projects/initial-utilities/wzip/.gdbinit
# Create on: Sun Sep  7 12:35:56 +01 2025

# run this script with:
# prompt> gdb -q

# layout split
tui enable

file ./wzip

break main

define skipops
	set $ino = $arg0
	while $ino > 0
		next
		set $ino = $ino - 1
	end
end

run ./tests/1.in
