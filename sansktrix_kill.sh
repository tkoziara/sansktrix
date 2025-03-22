#!/bin/bash

# Check if pkill is available
if command -v pkill &> /dev/null; then
    pkill -f '.sansktrix.'
else
    # Fallback to using ps and kill if pkill is not available
    pids=$(ps aux | grep '.sansktrix.' | grep -v grep | awk '{print $2}')
    if [[ -n "$pids" ]]; then
        echo "$pids" | xargs kill -9
    fi
fi