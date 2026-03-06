#include "security.h"
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>

// 1. System Call Filtering (Seccomp) - Week 8 Core
// This code is returned as a string and injected into the user's source code.
std::string SecurityModule::getSeccompPreamble() {
    return R"(
#include <sys/resource.h>
#include <sys/prctl.h>
#include <linux/seccomp.h>
#include <linux/filter.h>
#include <linux/audit.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <cstddef>  // <--- ADDED THIS LINE (Fixes offsetof error)

// INTERNAL SECURITY ENFORCEMENT
// Runs automatically before main() starts.
__attribute__((constructor))
void __enforce_week8_security() {
    
    // A. Memory Isolation Validation
    // Hard limit: 64MB. 
    struct rlimit memLimit = {64 * 1024 * 1024, 64 * 1024 * 1024};
    setrlimit(RLIMIT_AS, &memLimit);

    // B. Seccomp-BPF Filter (System Call Whitelist)
    struct sock_filter filter[] = {
        // 1. Load Architecture & Verify x86_64
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, (offsetof(struct seccomp_data, arch))),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, AUDIT_ARCH_X86_64, 1, 0),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL),

        // 2. Load Syscall Number
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, (offsetof(struct seccomp_data, nr))),

        // --- ALLOWED SYSCALLS ---
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_brk, 0, 1), BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_mmap, 0, 1), BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_munmap, 0, 1), BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_mprotect, 0, 1), BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
        
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_write, 0, 1), BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_read, 0, 1), BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_fstat, 0, 1), BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
        
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_exit, 0, 1), BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_exit_group, 0, 1), BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),

        // --- DEFAULT ACTION: KILL ---
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL),
    };

    struct sock_fprog prog = {
        .len = (unsigned short)(sizeof(filter) / sizeof(filter[0])),
        .filter = filter,
    };

    // Install Filter
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1) exit(31);
    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog) == -1) exit(31);
}
    )";
}

// 2. File System Isolation (Week 8)
// Creates a restricted folder 'sandbox_jail' and copies the binary there.
std::string SecurityModule::setupJail(const std::string& originalBinary) {
    std::string jailDir = "sandbox_jail";
    mkdir(jailDir.c_str(), 0755);
    std::string jailedBinary = jailDir + "/runner";

    std::ifstream src(originalBinary, std::ios::binary);
    std::ofstream dst(jailedBinary, std::ios::binary);
    dst << src.rdbuf();
    src.close();
    dst.close();
    chmod(jailedBinary.c_str(), 0755);
    return jailedBinary;
}

void SecurityModule::cleanupJail(const std::string& jailPath) {
    remove(jailPath.c_str()); 
    rmdir("sandbox_jail");    
}