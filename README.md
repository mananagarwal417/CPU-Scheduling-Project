# CPU Scheduling Project

A C++ implementation of various CPU scheduling algorithms used in operating systems.

## Overview

This project implements different CPU scheduling algorithms to demonstrate how operating systems manage process execution on a single CPU. These algorithms are fundamental to understanding operating system design and resource management.

## Features

- Implementation of multiple scheduling algorithms
- Comparison and analysis of different scheduling strategies
- Educational examples demonstrating process management

## Supported Algorithms

This project includes implementations of classic CPU scheduling algorithms:

- **FCFS** (First Come First Served)
- **SJF** (Shortest Job First)
- **Priority Scheduling**
- **Round Robin**
- And more...

## Prerequisites

- C++ compiler (C++11 or later)
- Linux/Unix environment or Windows with appropriate build tools

## Building the Project

```bash
g++ -o scheduler *.cpp
```

## Usage

Run the compiled executable:

```bash
./scheduler
```

Follow the on-screen prompts to select a scheduling algorithm and input process information.

## Project Structure

```
CPU-Scheduling-Project/
├── README.md
├── *.cpp          # C++ source files implementing scheduling algorithms
└── *.h            # Header files with algorithm definitions
```

## Learning Outcomes

After working with this project, you'll understand:

- How different scheduling algorithms work
- The trade-offs between various scheduling strategies
- Performance metrics like average waiting time and turnaround time
- How operating systems manage process execution

## Contributing

Contributions are welcome! Feel free to:
- Submit bug reports
- Suggest improvements
- Add new scheduling algorithms

## License

This project is provided as-is for educational purposes.

## Author

Created by: Ananya Mangal (@mananagarwal417)

---

**Note**: This is an educational project designed to understand CPU scheduling concepts. For production systems, refer to actual operating system implementations and their scheduling mechanisms.
