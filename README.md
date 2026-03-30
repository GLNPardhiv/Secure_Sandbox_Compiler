# AI-Assisted Secure Sandbox Compiler

**Project:** Compiler Design (CS1202)
**Student:** G. L. N. Pardhiv (Roll No: 24CSB0B24)  
**Current Phase:** Final Submission (Week 14)  

## 📌 Project Overview

DefenseOS is an enterprise-grade, 4-tier C++ execution environment designed to safely compile, analyze, and execute highly untrusted code. It emulates the core security architecture of automated grading platforms (like LeetCode or HackerRank) but introduces a novel hybrid approach: combining strict kernel-level constraints with Large Language Model (LLM) semantic intent analysis.

Unlike standard Docker-based sandboxes, DefenseOS leverages native Linux kernel APIs (`seccomp-bpf`, namespaces, `rlimit`) to achieve near-zero latency isolation. Furthermore, it features an integrated AI Tutor that translates opaque kernel signals and compiler syntax errors into actionable, educational explanations for the user.

## 🚀 Key Features

* **🧠 Tier 1 & 2 - Hybrid Threat Routing:** A C++ heuristic engine fast-passes safe code for zero latency, while intelligently routing obfuscated/ambiguous code to a Google Gemini AI agent to analyze semantic intent.
* **🛡️ Tier 3 - Kernel-Level Sandboxing:** Utilizes Linux `seccomp` to strictly whitelist system calls, mathematically guaranteeing the host is air-gapped from network and process-creation attacks.
* **⚡ Resource Exhaustion Protection:** Enforces strict POSIX `setrlimit` constraints (CPU time, RAM allocation, File Descriptors) to instantly terminate Denial of Service (DoS) attacks.
* **🤖 Tier 4 - AI Crash Diagnostics:** Intercepts raw POSIX signals (e.g., Signal 11, Signal 31) and utilizes LLMs to explain the exact cause of the memory/sandbox violation to the student.
* **🌐 Telemetry Dashboard:** A modern React/Vite frontend featuring a VS Code-style editor and a real-time visualizer that tracks code as it flows through the 4-tier security pipeline.

## 🛠️ Technical Architecture (The 4-Tier Pipeline)

The system operates on a highly synchronized Full-Stack pipeline:

* **The Client (React/Vite):** Captures code and provides real-time telemetry visualization.
* **The Gateway (Node.js/Express):** Manages asynchronous HTTP requests, provisions ephemeral cryptographic file names, and handles execution timeouts.
* **The Orchestrator (C++):** The core `sandboxcc` binary. It manages compilation, initializes the `clone()` namespaces, and applies the Berkeley Packet Filters (BPF).
* **The Intelligence Layer (Python):** Invoked dynamically via `popen()`, it acts as the bridge to the Google Gemini API for intent analysis and post-mortem diagnostics.

## ⚙️ Installation & Setup

### Prerequisites

* **OS:** Linux (Ubuntu 20.04+ recommended) - Required for native `<sys/resource.h>` & `<linux/seccomp.h>` support.
* **Compiler:** `g++` (GCC)
* **Runtime:** Node.js & npm (v18+)
* **Python:** Python 3 and pip

### 1. Install System Dependencies
```bash
sudo apt update && sudo apt install -y build-essential libseccomp-dev nodejs npm python3 python3-pip
```

### 2. Install Python Dependencies
```
pip3 install requests python-dotenv
```

### 3. Environment Configuration
Create a .env file in the root directory and add your Google Gemini API Key:
```
API_KEY=your_gemini_api_key_here
```

### 4. Compile the C++ Orchestrator
```
g++ sandboxcc.cpp compiler.cpp sandbox.cpp reporter.cpp security.cpp -o sandboxcc -lseccomp
```

### 5. Start the Application (Two Terminals Required)
Terminal 1 (Backend API):
```
npm install
node server.js
```

Terminal 2 (Frontend React UI):
```
cd frontend
npm install
npm run dev
```
Access the UI: Open your browser and navigate to http://localhost:5173.

## 🛡️ Security Mechanisms Explained

| Threat Vector | Defense Mechanism | Implementation File |
| :--- | :--- | :--- |
| **Obfuscated Malware** | Tier 2 LLM Semantic Intent Analysis | `risk_analyzer.py` |
| **Infinite CPU Loops** | `RLIMIT_CPU` (Hard limit: 2 seconds) | `sandbox.cpp` |
| **Heap Memory Bombs** | `RLIMIT_AS` (Max RAM: 256MB) | `sandbox.cpp` |
| **File Descriptor Leaks** | `RLIMIT_NOFILE` (Max FD: 64) | `sandbox.cpp` |
| **Remote Reverse Shells** | Default-Deny Seccomp (Blocks `execve`, `socket`) | `security.cpp` |
| **Process Table Exhaustion** | System call block (`fork`, `vfork`) & `CLONE_NEWPID` | `security.cpp` |

📂 Project Structure

    sandboxcc.cpp: Main C++ entry point, heuristic router, and execution orchestrator.
    compiler.cpp: Safely handles g++ invocation and error capture.
    sandbox.cpp: Manages clone() namespaces, pipes, and POSIX signal translation.
    security.cpp: Contains the Seccomp-BPF whitelist bytecode generation.
    risk_analyzer.py: The Python bridge to Google Gemini, handling API Backoff and JSON schemas.
    server.js: Node.js Express Gateway API.
    /frontend: The Vite/React application and telemetry dashboard.
    
License: MIT

Course: Compiler Design Lab (Final Submission)
