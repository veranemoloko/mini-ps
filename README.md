# PS
A simple process monitoring tool that displays running processes with their CPU and memory usage.

## Features
- Lists all running processes with their PID, username, process name, CPU time, and memory usage
- Parses Linux `/proc` filesystem to gather process information
- Caches system page size for memory calculations

### Build:
```bash
make
./minips
```