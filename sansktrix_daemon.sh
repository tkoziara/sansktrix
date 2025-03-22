#!/bin/bash

# sansktrix_daemon.sh: A daemon script that monitors terminal inactivity
# and launches the sansktrix terminal screen saver when the terminal
# has been inactive for a specified period of time.

# Path to sansktrix executable
SANSKTRIX_PATH="$HOME/sansktrix/sansktrix"

# Default inactivity timeout in seconds
INACTIVITY_TIMEOUT=60 

# Log file
LOG_FILE="/tmp/sansktrix_daemon.log"

# Check if log file already exists and delete it if so
if [[ -f "$LOG_FILE" ]]; then
    rm "$LOG_FILE"
fi

# Daemonization step
if [[ "$1" != "--daemon" ]]; then
    TERMINAL=$(tty | sed 's#^/dev/##') # Record terminal prior to #daemonization
    # Daemonize itself and pass terminal variable to itself
    setsid "$0" --daemon "$TERMINAL" </dev/null &>>"$LOG_FILE" &
    exit 0
fi

shift  # Remove "--daemon"
TERMINAL="$1" # Restore terminal variable

# Main loop
while true; do
    if [ ! -c "/dev/$TERMINAL" ]; then
        echo "[$(date +'%Y-%m-%d %H:%M:%S')] Terminal /dev/$TERMINAL gone. Exiting." >> "$LOG_FILE"
        exit 0
    fi

    # Get last INPUT (access time) and OUTPUT (modify time)
    last_input=$(stat -c %X "/dev/$TERMINAL")  # Last keyboard activity
    last_output=$(stat -c %Y "/dev/$TERMINAL") # Last terminal output
    current_time=$(date +%s)

    # Calculate idle time for input and output
    idle_input=$((current_time - last_input))
    idle_output=$((current_time - last_output))

    # Use the SMALLER of the two idle times
    idle_seconds=$(( idle_input < idle_output ? idle_input : idle_output ))

    # Determine activity status
    if (( idle_seconds > INACTIVITY_TIMEOUT )); then
        status="asleep"
    else
        status="active"
    fi

    if [[ "$status" == "asleep" ]]; then
        # Log going asleep
        echo "[$(date +'%Y-%m-%d %H:%M:%S')] Terminal is $status (Idle: ${idle_seconds}s)" >> "$LOG_FILE"

        # Run screen saver
        setsid "$SANSKTRIX_PATH" </dev/$TERMINAL >/dev/$TERMINAL 2>&1

        # Restore broken command prompt (TODO: understand why terminal prompt is not properly restored when sansktrix exits,  )
        #                               (      and/or possibly find a more elegant way of restoring prompt using $PS1 variable)
        printf '\n\e[1;32m%s@%s\e[0m:\e[1;34m%s\e[0m$ ' "$(whoami)" "$(hostname)" "$(pwd | sed "s|^$HOME|~|")" > /dev/$TERMINAL
    fi

    sleep 1
done