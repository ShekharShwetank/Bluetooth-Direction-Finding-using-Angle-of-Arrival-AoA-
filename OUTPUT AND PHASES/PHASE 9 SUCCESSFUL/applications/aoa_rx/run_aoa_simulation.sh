#!/bin/bash
set -e

echo "Starting AoA Multi-Device Simulation..."

cd ~/Desktop/LVLALPHA/nrf5340_project/tools/bsim/bin

# Start BabbleSim physical layer (2 devices, 60 seconds)
./bs_2G4_phy_v1 -s=aoa_test -D=2 -sim_length=60e6 &
PHY_PID=$!

sleep 1

# Start AoA transmitter (device 0)
./aoa_tx.exe -s=aoa_test -d=0 &
TX_PID=$!

# Start AoA receiver (device 1)
./aoa_rx.exe -s=aoa_test -d=1 &
RX_PID=$!

echo "Simulation running... (60 seconds)"
wait $PHY_PID $TX_PID $RX_PID
echo "AoA simulation completed successfully"
