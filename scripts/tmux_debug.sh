#!/usr/bin/sh

# Format the code
# find . -regex '.*\.\(cpp\|hpp\|cu\|cuh\|c\|h\)' -exec clang-format -style=file -i {} \;

rm -rf build

mkdir build

cd build

cmake -DCMAKE_BUILD_TYPE=Debug ..

make -j4

project_elf=$(ls | grep .elf -m 1)

cd ..

# Create the tmux session
tmux new-session -d -s pico_debug

# Create a pane for OpenOCD 
# NOTE: Check the README for information regarding openocd.
# AS of Dec 10, 24: Openocd must be built from source.
tmux send-keys -t pico_debug "sudo openocd -s tcl -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c \"adapter speed 5000\"" Enter

# Create a pane for minicom
tmux split-window -h -t pico_debug

#NOTE: For some reason, this command fails to execute during the script. Not sure why.
tmux send-keys -t pico_debug "minicom -b 115200 -o -D /dev/ttyACM0"

# Select the openocd window
tmux select-pane -t pico_debug:0.0

# Build and upload the project to the Pico
tmux split-window -v -p 50 -t pico_debug

tmux send-keys -t pico_debug "cd build" Enter
tmux send-keys -t pico_debug "gdb-multiarch $project_elf" Enter

# connect gdb to openocd
tmux send-keys -t pico_debug "target remote localhost:3333" Enter
tmux send-keys -t pico_debug "y" Enter  # NOTE: remove if not needed.

# load the elf file to pico
tmux send-keys -t pico_debug "load" Enter

# reset the pico and gdb
tmux send-keys -t pico_debug "monitor reset init" Enter

# set a breakpoint at the top of main
tmux send-keys -t pico_debug "break main" Enter

# continue to that break point
tmux send-keys -t pico_debug "continue" Enter

#NOTE: This is a workaround for minicom/tmux issues.
# This lets the user simply hit Enter to start the serial monitor
tmux select-pane -t pico_debug:0.2

# Attach to the debug session
tmux attach -t pico_debug

# When finished debugging, kill the session.
tmux kill-session -t pico_debug

clear
