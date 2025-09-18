# ostep-projects/processes-shell/.gdbinit
# Created on: Thu Sep 18 15:38:23 +01 2025
# Debugging Suite: Assembly Tracer
# Minimalist, expressive, and pedagogically tuned

# Core Setup
set pagination off
set confirm off
set verbose on
# set disassembly-flavor intel

# TUI Layout
tui enable
layout split
layout regs

# Load Binary & Symbols
# file <elf>
# break main

# Launch & Step
# run
# si

define jumper
	set $jump = $arg0
	while $jump > 0
		next
		set $jump = $jump - 1
	end
end

# Optional: Logic Anchors
# Replace with actual addresses or labels
# info address poploop
# info address Epilog
# break *0x4011a0    # ⬅️ Entry to character comparison loop
# break *0x4011f0    # ⬅️ Exit or cleanup logic

# Register Watchpoints (Optional)
# watch $rsi         # ⬅️ Index pointer
# watch $al          # ⬅️ Current character
# watch $dl          # ⬅️ Mirror character

# Inline Notes (for teaching or review)
# Use `display` to track values live
# display/i $pc
# display $rsi
# display $al
# display $dl

# Memory Sniffing Cheat Sheet (x/FMT)
# Format: x/NFU ADDR
#   N = repeat count
#   F = data format
#   U = unit size

# Format (F):
#   i = instruction
#   x = hex
#   d = decimal
#   u = unsigned decimal
#   o = octal
#   t = binary
#   c = char
#   f = float

# Unit Size (U):
#   b = byte (1 byte)        ← like `char`
#   h = halfword (2 bytes)   ← like `short`
#   w = word (4 bytes)       ← like `int`
#   g = giant (8 bytes)      ← like `long long` or `double`

# Examples:
#   x/16xb $rsp      # 16 bytes in hex
#   x/4hw 0x401000   # 4 halfwords in hex
#   x/8gc $rdi       # 8 chars from $rdi
#   x/1i $rip        # 1 instruction at $rip

# Register Inspection Tips
#   print $rax       # Show value of rax
#   p/x $rsi         # Print rsi in hex
#   p/d $al          # Print al in decimal
#   p/c $dl          # Print dl as char
#   info registers   # Dump all registers
#   info register rdi rsi rdx  # Selective dump

# Aliases & Macros (Commented Out)
# define sniff
#   x/16xb $rsp
#   x/8gc $rdi
#   x/8gc $rsi
# end

# define regs
#   info registers
#   display $rax
#   display $rdi
#   display $rsi
# end

# define steptrace
#   si
#   display/i $pc
#   display $al
#   display $dl
# end

# Cleanup (Optional)
# delete breakpoints
# clear

