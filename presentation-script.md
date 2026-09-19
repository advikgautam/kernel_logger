# Presentation Script & Slide Guide: Exploring Ring 0

## Slide 1: Title Slide
**Visual Content:**
* Project Title: Exploring Ring 0: System State Logging via Loadable Kernel Modules
* Presenters: Advik Gautam & [Team Members]
* Date: September 20, 2026

**Speaker Notes:**
"Welcome everyone. Today, our team will be diving into the lowest levels of the Linux operating system. We are going to explore Ring 0—the kernel space—by building, compiling, and testing a Loadable Kernel Module. Instead of just talking about operating system theory, we built a functional Kernel State Logger to demonstrate how software can directly interact with the core of the OS."

---

## Slide 2: The OS Divide: User Space vs. Kernel Space
**Visual Content:**
* Diagram showing Ring 3 (User Space) at the top and Ring 0 (Kernel Space) at the bottom.
* A barrier between them labeled "System Calls".

**Speaker Notes:**
"To understand this project, we first have to understand how modern operating systems protect themselves. Your computer's memory is divided into privilege rings. Normal applications, like web browsers, games, or standard C programs, run in Ring 3, known as User Space. They are sandboxed. If a User Space program crashes, it only takes down itself. It cannot directly touch hardware or internal OS memory. To do anything meaningful, User Space must ask the kernel for permission using System Calls. Our project bypasses this entirely."

---

## Slide 3: What is a Loadable Kernel Module (LKM)?
**Visual Content:**
* Definition of an LKM.
* Comparison: Dynamic Loading vs. Recompiling the OS.

**Speaker Notes:**
"So, how do we get code into Ring 0? We use a Loadable Kernel Module, or LKM. An LKM is a piece of compiled code that can be injected directly into the running kernel on demand, without needing to reboot the system. In the early days of Linux, if you wanted to add a new hardware driver, you had to recompile the entire operating system from scratch. LKMs solved this by making the kernel modular. You can plug and unplug features seamlessly."

---

## Slide 4: Project Objective
**Visual Content:**
* Goal: Write a program that bypasses User Space entirely.
* Introduction to the "Kernel State Logger".

**Speaker Notes:**
"Our objective was to write a program that operates completely inside Kernel Space, interacting with the system's most protected internal variables. We built the 'Kernel State Logger'. This module doesn't rely on standard libraries like the ones you use in regular C programming. Instead, it reads the kernel's internal hardware timers to calculate the exact system uptime when it is loaded, and — as you'll see in a moment — now also enumerates every running process, tracking exactly how long it remains active in the kernel before being removed."

---

## Slide 5: The Mechanics of Kernel Time (jiffies and HZ)
**Visual Content:**
* Definition of `jiffies`.
* Definition of `HZ`.
* The core formula: `jiffies / HZ = system uptime in seconds`.

**Speaker Notes:**
"To calculate time inside the kernel, we cannot use standard functions like `time.h`. We have to read the raw hardware data. The kernel tracks time using a global variable called `jiffies`. This is essentially a counter that increments every single time the system's hardware timer triggers an interrupt. The frequency of those interrupts is defined by a constant called `HZ`. If `HZ` is 250, it means the timer ticks 250 times a second. Therefore, by dividing the current `jiffies` by `HZ`, we get the exact number of seconds the system has been powered on."

---

## Slide 6: LKM Architecture (The Entry & Exit Points)
**Visual Content:**
* Contrast with standard C (`main()`).
* Explanation of `module_init()`.
* Explanation of `module_exit()`.

**Speaker Notes:**
"Architecturally, kernel modules do not execute like normal programs. There is no `main()` function. Instead, an LKM is event-driven. We define two primary macros: `module_init()` and `module_exit()`. The initialization function is triggered the exact millisecond the module is injected into the kernel. It sets up the module and runs our initial logic. The exit function sits dormant until the system administrator actively removes the module, at which point it cleans up memory and safely shuts the module down."

---

## Slide 7: Code Walkthrough: Initialization
**Visual Content:**
* Code snippet of the initialization function.

```c
static int __init logger_init(void) {
    unsigned long uptime_seconds = jiffies / HZ;
    start_jiffies = jiffies;

    printk(KERN_INFO "LOGGER: Module loaded.\n");
    printk(KERN_INFO "LOGGER: System uptime is %lu seconds.\n", uptime_seconds);

    list_processes();

    return 0;
}
```

**Speaker Notes:**
"Here is the exact code that runs when our module enters Ring 0. We first calculate the system uptime using the formula we discussed. We also save the current `jiffies` value into a variable so we have a starting timestamp. Because we are in the kernel, we cannot use `printf` to print to the terminal. Instead, we use `printk`, which writes our messages directly into the kernel's internal ring buffer. Finally, it calls our new `list_processes()` function, which we'll cover on the next slide."

---

## Slide 8: New Addition — Walking the Kernel's Process List
**Visual Content:**
* Code snippet of the `list_processes()` function.
* Label: "Phase 2: From reading one variable to traversing a live kernel data structure."

```c
static void list_processes(void) {
    struct task_struct *task;

    printk(KERN_INFO "LOGGER: --- Active Process List ---\n");

    for_each_process(task) {
        printk(KERN_INFO "LOGGER: PID: %d | Name: %s\n", task->pid, task->comm);
    }

    printk(KERN_INFO "LOGGER: --- End of Process List ---\n");
}
```

**Speaker Notes:**
"This is what we've added since our last checkpoint. Every running program on the system is represented inside the kernel by a data structure called `task_struct`, and the kernel keeps all of them linked together in a live process list. We used `for_each_process()`, a standard kernel macro, to walk that entire list and log each process's PID and name. This matters beyond just being a neat feature — this exact list is what legitimate security tools read to detect what's running, and it's also the exact list that rootkits try to quietly tamper with to hide malicious processes from detection. So this update moves us from reading a single passive variable to actively traversing a live, security-relevant kernel structure."

---

## Slide 9: Code Walkthrough: Cleanup

**Visual Content:**

* Code snippet of the exit function.

```c
static void __exit logger_exit(void) {
    unsigned long active_jiffies = jiffies - start_jiffies;
    unsigned long active_seconds = active_jiffies / HZ;

    printk(KERN_INFO "LOGGER: Module unloaded.\n");
    printk(KERN_INFO "LOGGER: Module was active for %lu seconds.\n", active_seconds);
}
```

**Speaker Notes:**
"When the module is removed, this exit function triggers. We take the current `jiffies` value and subtract our original starting timestamp. This gives us the exact number of hardware ticks that occurred while our module was alive. We divide that by `HZ` again to convert it back into seconds, and log the final lifespan of the module before it vanishes from memory."

---

## Slide 10: The Build Process (Kbuild & Makefiles)

**Visual Content:**

* Explanation of kernel headers and Kbuild.
* Makefile snippet.

```makefile
obj-m += logger.o

all:
	make LLVM=1 -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make LLVM=1 -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

**Speaker Notes:**
"Compiling kernel code is fundamentally different from normal C development. You cannot just run `gcc`. You have to hook into the Linux Kbuild system. Our Makefile points to the specific kernel headers located in `/lib/modules/`. One unique challenge we faced during development on our CachyOS system was a compiler mismatch. The kernel was built with Clang, but our system defaulted to GCC. We solved this by explicitly passing the `LLVM=1` flag into our Makefile to ensure perfectly aligned architecture during compilation."

---

## Slide 11: Execution Workflow (The Commands)

**Visual Content:**

* `make`
* `sudo insmod logger.ko`
* `sudo rmmod logger`

**Speaker Notes:**
"Once compiled into a `.ko` (Kernel Object) file, we use specific terminal commands to manage the module. `insmod` inserts the module into the running kernel, and `rmmod` removes it. Notice that both commands require `sudo`. This is the OS protecting itself—a normal user cannot arbitrarily inject code into Ring 0. Only the root administrator has the authority to bridge the gap between User Space and Kernel Space."

---

## Slide 12: Security Implications & Risks

**Visual Content:**

* No safety nets in Ring 0.
* Kernel Panics vs. App Crashes.
* Introduction to Rootkits.

**Speaker Notes:**
"Writing kernel code carries extreme risk. Because there is no sandbox, there is no safety net. If you write a standard program and make a memory error, the app closes. If you make a memory error in a kernel module, it triggers a Kernel Panic, instantly crashing the entire computer and requiring a physical reboot. From an offensive security perspective, advanced malware known as Rootkits use this exact same LKM architecture — and often the exact same process-list traversal we just showed — to lie to the operating system, hiding their files and processes from all antivirus software."

---

## Slide 13: Conclusion & Output

**Visual Content:**

* Screenshot of the terminal running `dmesg | tail`.
* Visual highlight of the uptime, process list, and unload logs generated by the module.

**Speaker Notes:**
"To prove the module worked, we use the `dmesg` command to read the kernel's internal ring buffer. As you can see in the screenshot, our module successfully logged its entry, accurately calculated the system uptime directly from hardware timers, walked and logged the live process list, and then tracked exactly how many seconds it survived in the kernel before we removed it. It is a clean, highly privileged execution of C code at the lowest level of the OS."

---

## Slide 14: Roadmap — What's Next (Phase 3)

**Visual Content:**
* Phase 1: Uptime logging (done)
* Phase 2: Process list enumeration (done — today's update)
* Phase 3: Live process monitoring via `kprobes` (planned)

**Speaker Notes:**
"Right now, our module only checks the process list once — the exact moment it's loaded, like taking a single photograph. Our next planned step is to use a kernel feature called `kprobes`, which lets us hook into the process-creation event itself, so the module logs every new process the instant it starts — turning our one-time snapshot into live, continuous monitoring. That's the same underlying mechanism basic antivirus and EDR software use to watch a system in real time, and it's the direction we're taking this project next. Thank you for listening, we'll now take any questions."
