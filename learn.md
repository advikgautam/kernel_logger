# The Ultimate Beginner's Guide to Linux Kernels and Modules 🧠

Welcome to your study guide! If you are new to Linux, kernels, or C programming, all the technical jargon can sound like an alien language. This document will explain everything from the ground up, using simple analogies like we are talking about a restaurant.

## 🍔 Part 1: The Absolute Basics

### What is an Operating System (OS)?

Think of your computer as a massive restaurant. The **Hardware** (CPU, RAM, Hard Drive) are the kitchen appliances (stoves, fridges, blenders). The **Applications** (Chrome, Discord, your terminal) are the customers sitting at the tables.
An Operating System is the entire restaurant building, including the staff and the rules.

### What is the "Kernel"?

The Kernel is the **Head Chef** and **Kitchen Manager**.
Customers (Applications) are not allowed to just walk into the kitchen and touch the stoves (Hardware). If a customer wants food (needs to save a file, use the internet, or draw on the screen), they must give an order to a waiter, who hands it to the Head Chef. The Head Chef (Kernel) safely uses the stoves and brings the result back.

This strict separation is why a broken web browser usually just crashes the browser, not your whole computer.

* **User Space (Ring 3):** The dining room. Where your normal apps live. It's safe here.

* **Kernel Space (Ring 0):** The kitchen. Only the Kernel and drivers live here. It has absolute power. If something breaks here, the whole restaurant burns down (Kernel Panic / System Crash).

### What is a Kernel Module (`.ko` file)?

In the old days, if you wanted to teach the Head Chef a new recipe (add a new driver or feature), you had to close the restaurant, teach him, and reopen (reboot the computer).
A **Kernel Module (LKM - Loadable Kernel Module)** is like a temporary guest chef. You can bring them into the kitchen while the restaurant is running, let them do their job, and kick them out when you're done, all without restarting the computer!

## 🛠️ Part 2: Breaking Down Our C Code (`logger.c`)

When we wrote our C code, we wrote a script for our guest chef. Let's look at what the weird words mean:

* `#include <linux/module.h>`: This is us giving our guest chef the standard kitchen toolbelt. It contains the basic definitions needed to exist in the kernel.

* `jiffies`: The kernel has an internal heartbeat. A "jiffy" is one single tick of that heartbeat. It counts up forever from the moment you turn your PC on.

* `HZ` (Hertz): This is how many heartbeats happen in exactly one second.

* **The Math:** If we take the total heartbeats (`jiffies`) and divide by heartbeats-per-second (`HZ`), we get exactly how many seconds the computer has been turned on (Uptime)!

* `__init`: This tells the kernel, "Only run this code the exact second the module arrives."

* `__exit`: This tells the kernel, "Run this right as the module is getting kicked out."

* `printk`: In normal programming, you use `printf` to print text to the screen. The kernel doesn't have a screen! It lives in the background. So, `printk` (print kernel) writes messages to a secret diary called the **kernel ring buffer**.

## 🕵️ Part 3: New Addition — Reading the Process List

Once our guest chef is in the kitchen, we gave them a new task: read the restaurant's **guest book** before leaving.

### What's a "process"?

Every running program on your computer — your browser, a terminal, a game — is called a **process** while it's running. Your computer probably has 100-300 processes running right now, even if you only see a few windows open.

### How does the kernel track all of them?

The kernel keeps an internal list with one entry per running process. Each entry is a data structure called `task_struct`, and it holds that process's:

* `pid` — its ID number (every process gets a unique number)
* `comm` — its short name (e.g. `firefox`, `bash`, `systemd`)

Think of this like a guest book at the restaurant door — every customer currently inside is written down.

### The new code, explained line by line

* `#include <linux/sched.h>` and `#include <linux/sched/signal.h>`: These give us access to the `task_struct` definition and the tools to walk the process list — new toolbelt items for our guest chef.

* `struct task_struct *task;`: This creates a variable that will point at one guest-book entry at a time as we flip through the pages.

* `for_each_process(task) { ... }`: This is a ready-made kernel macro whose entire job is "start at the first entry in the guest book, and keep moving to the next one until you've seen them all." Each time through the loop, `task` points at a different running process.

* `printk(KERN_INFO "LOGGER: PID: %d | Name: %s\n", task->pid, task->comm);`: For each entry, we print its ID number and name into the kernel's diary.

### Why this matters (not just "because it's cool")

This exact guest book is a big deal in cybersecurity. Legitimate security tools (antivirus, EDR software) read this same list to check "is anything suspicious running?" On the flip side, a category of malware called **rootkits** tries to sneakily edit this same list so their own entry disappears — making the malicious process invisible to `ps`, Task Manager, and antivirus scans. So by simply reading this list, our module is touching one of the most fought-over pieces of the operating system.

## 🤖 Part 4: What is the `Makefile`?

A compiler is a translator. It turns human-readable C code into binary 1s and 0s that the computer understands.

* **What is `make`?** It's a build robot. Instead of typing a massive, complicated translation command every time, we write a `Makefile` (a recipe). When we type `make`, the robot reads the recipe and builds our module automatically.

* **Why `LLVM=1`?** Usually, Linux uses a translator called **GCC**. But your specific system (CachyOS) was built using a newer, faster translator called **Clang/LLVM**. By typing `LLVM=1`, we tell the robot, "Hey, make sure you use the exact same translator the Head Chef uses, otherwise they won't understand each other."

## 💻 Part 5: The Terminal Commands Explained

Here is exactly what you did in the terminal, step-by-step:

### 1. `mkdir -p ~/kernel_logger` & `cd ~/kernel_logger`

* `mkdir` (Make Directory) creates a new folder.

* `~` means your home folder (like `C:\Users\Advik` on Windows).

* `cd` (Change Directory) opens that folder so we can work inside it.

### 2. `make`

Wakes up the build robot to compile your `.c` code into a `.ko` (Kernel Object) file.

### 3. `sudo insmod logger.ko`

* `sudo` (SuperUser Do): Puts you in Boss Mode. Normal users can't mess with the kitchen. `sudo` proves you own the restaurant.

* `insmod` (Insert Module): Shoves your `logger.ko` file straight into the live kernel. This triggers your `__init` code — including the new process list walk!

### 4. `dmesg | tail -n 20`

* `dmesg` (Diagnostic Messages): This command prints out the Kernel's secret diary where `printk` writes.

* `|` (The Pipe): This takes the massive diary output and pipes it into the next command.

* `tail -n 20`: Says "I don't want to read the whole diary, just show me the bottom 20 lines." Since the process list adds many more log lines than before, we increased this from 5 to 20 so you can actually see the full guest book.

### 5. `sudo rmmod logger`

* `rmmod` (Remove Module): Kicks your guest chef out of the kitchen safely. This triggers your `__exit` code, which calculates how long the module was active.

## 🔭 Part 6: What's Next (Phase 3)

Right now, the module only reads the guest book **once** — the exact second it's loaded, like taking a single photo. The planned next step is to use a kernel feature called `kprobes`, which lets the module "ring a bell" every time a *new* process starts — switching from a single photo to a live security camera feed of the system.
