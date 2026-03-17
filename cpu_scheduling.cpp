#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <algorithm>
#include <iomanip>

using namespace std;

// ==========================================
// 1. Core Data Structure
// ==========================================
struct Process {
    int id;
    int arrivalTime;
    int burstTime;
    int remainingTime;
    int completionTime = 0;
    int turnaroundTime = 0;
    int waitingTime = 0;

    Process(int id, int at, int bt) : id(id), arrivalTime(at), burstTime(bt), remainingTime(bt) {}
};

// Global mutex to prevent threads from scrambling console output
mutex consoleMutex;


// ==========================================
// 2. Abstract Base Class (OOP)
// ==========================================
class Scheduler {
protected:
    string name;
    vector<Process> processes;
    int contextSwitches;
    double cpuUtilization;
    double avgTurnaroundTime;
    double avgWaitingTime;

public:
    Scheduler(string n, vector<Process> p) : name(n), processes(p), contextSwitches(0), cpuUtilization(0), avgTurnaroundTime(0), avgWaitingTime(0) {}
    virtual ~Scheduler() = default;

    // Pure virtual function enforcing implementation in derived classes
    virtual void simulate() = 0;

    void calculateMetrics(int totalTime) {
        int totalBurst = 0;
        double totalTAT = 0, totalWT = 0;
        
        // Find the earliest arrival time to calculate true total time
        int minArrival = processes.empty() ? 0 : processes[0].arrivalTime;
        for (const auto& p : processes) {
            minArrival = min(minArrival, p.arrivalTime);
        }
        
        int activeCPUTime = totalTime - minArrival;

        for (const auto& p : processes) {
            totalBurst += p.burstTime;
            totalTAT += p.turnaroundTime;
            totalWT += p.waitingTime;
        }
        
        // Avoid division by zero
        if (activeCPUTime > 0) {
            cpuUtilization = ((double)totalBurst / activeCPUTime) * 100.0;
        } else {
            cpuUtilization = 0;
        }
        
        if (!processes.empty()) {
            avgTurnaroundTime = totalTAT / processes.size();
            avgWaitingTime = totalWT / processes.size();
        }
    }

    void displayResults() {
        lock_guard<mutex> lock(consoleMutex);
        cout << "\n========================================\n";
        cout << "   Results for " << name << "\n";
        cout << "========================================\n";
        cout << "Avg Turnaround Time : " << fixed << setprecision(2) << avgTurnaroundTime << " ms\n";
        cout << "Avg Waiting Time    : " << fixed << setprecision(2) << avgWaitingTime << " ms\n";
        cout << "Context Switches    : " << contextSwitches << "\n";
        cout << "CPU Utilization     : " << fixed << setprecision(2) << cpuUtilization << " %\n";
    }
};


// ==========================================
// 3. FCFS Implementation
// ==========================================
class FCFSScheduler : public Scheduler {
public:
    FCFSScheduler(vector<Process> p) : Scheduler("First-Come, First-Serve (FCFS)", p) {}

    void simulate() override {
        sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
            return a.arrivalTime < b.arrivalTime;
        });

        int currentTime = 0;
        for (auto& p : processes) {
            if (currentTime < p.arrivalTime) {
                currentTime = p.arrivalTime; // CPU sits idle until next process arrives
            }
            currentTime += p.burstTime;
            p.completionTime = currentTime;
            p.turnaroundTime = p.completionTime - p.arrivalTime;
            p.waitingTime = p.turnaroundTime - p.burstTime;
            contextSwitches++;
        }
        
        contextSwitches = max(0, contextSwitches - 1); // First load isn't a switch
        calculateMetrics(currentTime);
        displayResults();
    }
};


// ==========================================
// 4. SJF Implementation (Non-Preemptive)
// ==========================================
class SJFScheduler : public Scheduler {
public:
    SJFScheduler(vector<Process> p) : Scheduler("Shortest Job First (SJF)", p) {}

    void simulate() override {
        int currentTime = 0;
        int completed = 0;
        int n = processes.size();
        vector<bool> isCompleted(n, false);

        while (completed != n) {
            int shortestIdx = -1;
            int minBurst = 1e9;

            for (int i = 0; i < n; i++) {
                if (processes[i].arrivalTime <= currentTime && !isCompleted[i]) {
                    if (processes[i].burstTime < minBurst) {
                        minBurst = processes[i].burstTime;
                        shortestIdx = i;
                    }
                }
            }

            if (shortestIdx == -1) {
                currentTime++; // CPU is idle
            } else {
                contextSwitches++; 
                currentTime += processes[shortestIdx].burstTime;
                
                processes[shortestIdx].completionTime = currentTime;
                processes[shortestIdx].turnaroundTime = processes[shortestIdx].completionTime - processes[shortestIdx].arrivalTime;
                processes[shortestIdx].waitingTime = processes[shortestIdx].turnaroundTime - processes[shortestIdx].burstTime;
                
                isCompleted[shortestIdx] = true;
                completed++;
            }
        }
        
        contextSwitches = max(0, contextSwitches - 1);
        calculateMetrics(currentTime);
        displayResults();
    }
};


// ==========================================
// 5. Round Robin Implementation
// ==========================================
class RoundRobinScheduler : public Scheduler {
private:
    int timeQuantum;
public:
    RoundRobinScheduler(vector<Process> p, int tq) : Scheduler("Round Robin (RR)", p), timeQuantum(tq) {}

    void simulate() override {
        // Sort by arrival time initially
        sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
            return a.arrivalTime < b.arrivalTime;
        });

        int currentTime = 0;
        int completed = 0;
        int n = processes.size();
        queue<int> readyQueue;
        vector<bool> inQueue(n, false);
        
        int idx = 0;
        
        // Fast-forward time to the first arriving process if necessary
        if (n > 0 && processes[0].arrivalTime > currentTime) {
            currentTime = processes[0].arrivalTime;
        }

        // Push all processes that have arrived at the start time
        while (idx < n && processes[idx].arrivalTime <= currentTime) {
            readyQueue.push(idx);
            inQueue[idx] = true;
            idx++;
        }

        int lastProcessId = -1;

        while (completed != n) {
            if (readyQueue.empty()) {
                currentTime++; // CPU is idle
                while (idx < n && processes[idx].arrivalTime <= currentTime) {
                    readyQueue.push(idx);
                    inQueue[idx] = true;
                    idx++;
                }
                continue;
            }

            int currProcess = readyQueue.front();
            readyQueue.pop();
            
            // Count context switch if we change processes
            if (lastProcessId != processes[currProcess].id && lastProcessId != -1) {
                contextSwitches++;
            }
            lastProcessId = processes[currProcess].id;

            int timeSpent = min(processes[currProcess].remainingTime, timeQuantum);
            processes[currProcess].remainingTime -= timeSpent;
            currentTime += timeSpent;

            // Check for newly arrived processes DURING the execution block
            while (idx < n && processes[idx].arrivalTime <= currentTime) {
                readyQueue.push(idx);
                inQueue[idx] = true;
                idx++;
            }

            // If current process isn't done, put it back at the end of the queue
            if (processes[currProcess].remainingTime > 0) {
                readyQueue.push(currProcess);
            } else {
                // Process finished
                processes[currProcess].completionTime = currentTime;
                processes[currProcess].turnaroundTime = currentTime - processes[currProcess].arrivalTime;
                processes[currProcess].waitingTime = processes[currProcess].turnaroundTime - processes[currProcess].burstTime;
                completed++;
            }
        }
        
        calculateMetrics(currentTime);
        displayResults();
    }
};


// ==========================================
// 6. Main Execution (Multithreading)
// ==========================================
int main() {
    // Mock processes: {ID, Arrival Time, Burst Time}
    // Mixing up arrival times to prove the algorithms work dynamically
    vector<Process> processList = {
        Process(1, 0, 8),
        Process(2, 1, 4),
        Process(3, 2, 9),
        Process(4, 3, 5)
    };

    cout << "Starting Concurrent CPU Scheduling Simulation...\n";
    cout << "Process Count: " << processList.size() << "\n";

    // Create scheduler instances
    FCFSScheduler fcfs(processList);
    SJFScheduler sjf(processList);
    RoundRobinScheduler rr(processList, 3); // Time quantum of 3

    // Launch threads to run simulations concurrently
    thread t1(&FCFSScheduler::simulate, &fcfs);
    thread t2(&SJFScheduler::simulate, &sjf);
    thread t3(&RoundRobinScheduler::simulate, &rr);

    // Wait for all threads to finish execution
    t1.join();
    t2.join();
    t3.join();

    cout << "\nSimulation Complete. All threads joined successfully.\n";
    return 0;
}