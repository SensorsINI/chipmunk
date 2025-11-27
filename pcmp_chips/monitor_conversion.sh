#!/bin/bash
# Monitor conversion process to identify bottlenecks

echo "Monitoring conversion process..."
echo "Press Ctrl+C to stop"
echo ""

# Find klayout processes
KLAYOUT_PIDS=$(pgrep -f "klayout.*export_cif_png.rb" | head -1)

if [ -z "$KLAYOUT_PIDS" ]; then
    echo "No klayout conversion processes found. Waiting..."
    while [ -z "$KLAYOUT_PIDS" ]; do
        sleep 1
        KLAYOUT_PIDS=$(pgrep -f "klayout.*export_cif_png.rb" | head -1)
    done
fi

echo "Found klayout PID: $KLAYOUT_PIDS"
echo ""

# Monitor loop
while true; do
    if ! kill -0 $KLAYOUT_PIDS 2>/dev/null; then
        # Process ended, check for new one
        KLAYOUT_PIDS=$(pgrep -f "klayout.*export_cif_png.rb" | head -1)
        if [ -z "$KLAYOUT_PIDS" ]; then
            echo "No active conversion processes"
            sleep 2
            continue
        fi
    fi
    
    # Get process stats
    PS_STATS=$(ps -p $KLAYOUT_PIDS -o pid,pcpu,pmem,rss,vsz,state,etime,cmd --no-headers 2>/dev/null)
    
    # Get system stats
    VMSTAT=$(vmstat 1 2 | tail -1)
    
    # Parse vmstat output
    CPU_USER=$(echo $VMSTAT | awk '{print $13}')
    CPU_SYS=$(echo $VMSTAT | awk '{print $14}')
    CPU_IDLE=$(echo $VMSTAT | awk '{print $15}')
    CPU_WAIT=$(echo $VMSTAT | awk '{print $16}')
    IO_BI=$(echo $VMSTAT | awk '{print $9}')  # blocks in
    IO_BO=$(echo $VMSTAT | awk '{print $10}') # blocks out
    
    # Get disk I/O if available
    if command -v iostat >/dev/null 2>&1; then
        DISK_IO=$(iostat -x 1 2 2>/dev/null | tail -n +4 | head -1)
    else
        DISK_IO="N/A"
    fi
    
    # Clear line and print stats
    printf "\r\033[K"
    printf "CPU: User=%s%% Sys=%s%% Idle=%s%% Wait=%s%% | I/O: In=%s Out=%s | Process: %s" \
        "$CPU_USER" "$CPU_SYS" "$CPU_IDLE" "$CPU_WAIT" "$IO_BI" "$IO_BO" "$PS_STATS"
    
    sleep 2
done

