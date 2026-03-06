# AI-Assisted Secure Sandbox Compiler

> **Project:** Compiler Design (CS1202)  
> **Student:** G. L. N. Pardhiv (Roll No: 24CSB0B24)  
> **Current Phase:** Week 9 (Full Stack Integration)

## 📌 Project Overview
The **Secure Sandbox Compiler** is a robust C++ execution environment designed to compile and run untrusted code safely. It emulates the core functionality of online coding platforms (like LeetCode or HackerRank) by enforcing strict kernel-level security policies.

Unlike simple compiler wrappers, this system employs **Process Isolation**, **Resource Limits (RLIMITs)**, and **System Call Filtering (Seccomp-BPF)** to prevent malicious code from harming the host system. It also features an AI-driven error reporter that translates cryptic compiler errors into beginner-friendly explanations.

---

## 🚀 Key Features

*   **🛡️ Kernel-Level Sandboxing:** Uses Linux `seccomp` to strictly whitelist system calls.
*   **⚡ Resource Isolation:** Enforces strict limits on CPU time, Memory (RAM), and Output size.
*   **🔒 File System Jailing:** Executes code in a restricted, ephemeral directory (`sandbox_jail`).
*   **🤖 AI Error Analysis:** heuristics to explain *why* compilation failed (e.g., missing namespaces).
*   **🌐 Web Interface:** A Node.js + Express frontend to write and execute code in the browser.
*   **📊 JSON Reporting:** Structural output for easy integration with frontend applications.

---

## 🛠️ Technical Architecture

The system operates on a **Supervisor-Worker Model**:

1.  **Compiler (`compiler.cpp`):** Injects a security preamble (C++ Constructor) into the user's code before compilation.
2.  **Supervisor (`sandboxcc.cpp`):**
    *   Compiles the code.
    *   Sets up a **Jail Directory**.
    *   Spawns a child process.
3.  **Worker (Child Process):**
    *   **Drops Privileges:** Applies `RLIMIT_CPU` (1s), `RLIMIT_AS` (64MB), `RLIMIT_NPROC` (0).
    *   **Installs Filters:** Loads **BPF Bytecode** into the kernel to block `execve`, `socket`, `fork`, etc.
    *   **Executes:** Replaces itself with the user binary.
4.  **Auditor:** Logs all events to `sandbox_exec.log`.

---

## ⚙️ Installation & Setup

### Prerequisites
*   **OS:** Linux (Ubuntu/Debian recommended) - *Required for `<sys/resource.h>` & `<linux/seccomp.h>`*
*   **Compiler:** `g++` (GCC)
*   **Runtime:** Node.js & npm (For Web UI)

### Build Instructions
1.  **Clone the Repository:**
    ```bash
    git clone <your-repo-url>
    cd Sandbox_C
    ```

2.  **Compile the Sandbox Backend:**
    ```bash
    g++ sandboxcc.cpp compiler.cpp sandbox.cpp reporter.cpp security.cpp -o sandboxcc
    ```

3.  **Install Web Dependencies:**
    ```bash
    npm init -y
    npm install express body-parser
    ```

---

## 🖥️ Usage

### 1. Command Line Interface (CLI)
You can run the tool directly from the terminal for testing.

*   **Run a file safely:**
    ```bash
    ./sandboxcc test_files/test.cpp --run
    ```

*   **Get JSON Output (for APIs):**
    ```bash
    ./sandboxcc test_files/test.cpp --run --json
    ```

### 2. Web Interface (GUI)
To use the browser-based editor:

1.  Start the server:
    ```bash
    node server.js
    ```
2.  Open your browser and go to:
    `http://localhost:3000`

---

## 🛡️ Security Mechanisms Explained

| Threat Vector | Defense Mechanism | Implementation File |
| :--- | :--- | :--- |
| **Infinite Loops** | `RLIMIT_CPU` (Hard limit: 2s) | `sandbox.cpp` |
| **Memory Bombs** | `RLIMIT_AS` (Max RAM: 64MB) | `security.cpp` |
| **Fork Bombs** | `RLIMIT_NPROC` (0 child processes) | `sandbox.cpp` |
| **Disk Filling** | `RLIMIT_FSIZE` (Max Output: 1MB) | `sandbox.cpp` |
| **Remote Shells** | `Seccomp-BPF` (Blocks `execve`, `socket`) | `security.cpp` |
| **File Snooping** | Ephemeral Jail Directory | `security.cpp` |

---

## 📂 Project Structure

*   **`sandboxcc.cpp`**: Main entry point (CLI argument parsing).
*   **`compiler.cpp`**: Handles `g++` invocation and security code injection.
*   **`sandbox.cpp`**: Manages the `fork()`, pipes, and timeout logic.
*   **`security.cpp`**: Contains the BPF Bytecode generation and Jail setup.
*   **`reporter.cpp`**: Formats output (Text/JSON) and generates AI hints.
*   **`server.js`**: Middleware connecting the Web UI to the C++ backend.

---

**License:** MIT  
**Course:** Compiler Design Lab Week 9
