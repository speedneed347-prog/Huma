# Security Policy for HUMA X

## Memory Safety
- The HUMA X Virtual Machine enforces bounds checking on all list and array indexing operations. Out-of-bounds indices evaluate safely to `nothing` rather than causing undefined memory corruption.
- Values are managed through C++ standard library RAII reference containers (`std::shared_ptr`, `std::vector`, `std::map`), preventing memory leaks and dangling pointers.

## Process Isolation
- The Cloud Native Compiler API executes runs in bounded sub-processes with execution timeouts and memory limits.
- Untrusted user scripts are executed without raw system-level privilege escalation.

## Reporting Vulnerabilities
Please report vulnerabilities to `security@huma-lang.org`.
