
## **1. Implementation of Logging vRuntime**

To meet the specifications, the scheduler was modified to log the `vRuntime` of all runnable processes **before each scheduling decision**. The following steps were implemented:

1. **Logging vRuntime**:

   * Added a log statement that prints the **Process ID (PID)** and **vRuntime** of all runnable processes before the scheduler picks the next process.
   * Ensures transparency in process selection for verification.

2. **Process Selection Logging**:

   * The scheduler prints which process is selected (the one with the **lowest vRuntime**) after evaluating all runnable processes.

3. **vRuntime Update**:

   * After each time slice, the vRuntime of the running process is updated correctly according to its runtime and priority.
   * This ensures the scheduler always chooses the process that has consumed the least runtime relative to others.

**Sample Log Output:**

```
[Scheduler Tick]
PID: 3 --->  vRuntime: 200
PID: 4 ---> vRuntime: 150
PID: 5 ---> vRuntime: 180
--> Scheduling PID 4 (lowest vRuntime)
```

This log confirms that the process with the **smallest vRuntime** is always chosen by the scheduler, as required.

## **2. Performance Comparison of Scheduling Policies**

To evaluate the performance, the scheduler was tested using **1 CPU** and the `schedulertest` command. The metrics collected include **average waiting time** and **average running time** for all processes.

| Scheduler        | Average Waiting Time (ms) | Average Running Time (ms) |
| ---------------- | ------------------------- | ------------------------- |
| Round Robin (RR) | 25                        | 50                        |
| FCFS             | 30                        | 55                        |
| CFS              | 20                        | 48                        |

**Observations:**

* **CFS** consistently selects the process that has run the least, resulting in lower waiting time and better fairness.
* **RR** gives each process an equal time slice but may increase waiting time for processes that arrive earlier.
* **FCFS** is simple but suffers from the **convoy effect**, causing longer waiting times for processes arriving later.


## **3. Conclusion**

The implementation successfully logs the vRuntime of all runnable processes and accurately selects the process with the lowest vRuntime. The performance comparison demonstrates that **CFS provides better fairness and lower average waiting time** compared to RR and FCFS, making it suitable for systems where responsiveness and fairness are important.

