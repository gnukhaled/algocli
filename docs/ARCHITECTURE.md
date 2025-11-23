# AlgoBackup CLI - Architecture Documentation

## System Overview

AlgoBackup CLI is a command-line interface for managing a BTRFS-based backup appliance system. It provides administrative functions for backup point management, access control, file sharing, and system operations.

## Architecture Principles

The refactored architecture follows these principles:

1. **Security First**: All operations validated, sanitized, and executed safely
2. **Separation of Concerns**: Clear module boundaries and responsibilities
3. **Dependency Inversion**: High-level modules independent of low-level details
4. **Single Responsibility**: Each module has one clear purpose
5. **Fail-Safe Defaults**: Secure defaults, explicit actions for dangerous operations

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     User Interface Layer                     │
│  ┌────────────┐  ┌────────────┐  ┌───────────────────────┐  │
│  │  Readline  │  │   Prompt   │  │  Command Completion   │  │
│  └────────────┘  └────────────┘  └───────────────────────┘  │
└───────────────────────────┬─────────────────────────────────┘
                            │
┌───────────────────────────┴─────────────────────────────────┐
│                   Command Dispatcher Layer                   │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  Command Parser & Router                             │   │
│  │  - Parses command line                               │   │
│  │  - Routes to appropriate command module              │   │
│  │  - Handles help and completion                       │   │
│  └──────────────────────────────────────────────────────┘   │
└───────────────────────────┬─────────────────────────────────┘
                            │
┌───────────────────────────┴─────────────────────────────────┐
│                     Command Module Layer                     │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────┐  │
│  │   Backup     │  │    Access    │  │       NFS        │  │
│  │    Points    │  │   Control    │  │     Shares       │  │
│  └──────────────┘  └──────────────┘  └──────────────────┘  │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────┐  │
│  │  Filesystem  │  │    System    │  │       Log        │  │
│  └──────────────┘  └──────────────┘  └──────────────────┘  │
└───────────────────────────┬─────────────────────────────────┘
                            │
┌───────────────────────────┴─────────────────────────────────┐
│                      Business Logic Layer                    │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  Input Validation                                    │   │
│  │  - Alphanumeric validation                           │   │
│  │  - Path component validation                         │   │
│  │  - IP address validation                             │   │
│  │  - Port validation                                   │   │
│  └──────────────────────────────────────────────────────┘   │
└───────────────────────────┬─────────────────────────────────┘
                            │
┌───────────────────────────┴─────────────────────────────────┐
│                      Service Layer                           │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────┐  │
│  │   Database   │  │ Safe Exec    │  │  Configuration   │  │
│  │   Access     │  │  Wrapper     │  │    Manager       │  │
│  └──────────────┘  └──────────────┘  └──────────────────┘  │
│  ┌──────────────┐  ┌──────────────┐                        │
│  │   Logging    │  │     Auth     │                        │
│  └──────────────┘  └──────────────┘                        │
└───────────────────────────┬─────────────────────────────────┘
                            │
┌───────────────────────────┴─────────────────────────────────┐
│                    External Interface Layer                  │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────┐  │
│  │   SQLite     │  │   Systemd    │  │      BTRFS       │  │
│  │   Database   │  │  (systemctl) │  │   File System    │  │
│  └──────────────┘  └──────────────┘  └──────────────────┘  │
│  ┌──────────────┐  ┌──────────────┐                        │
│  │     PAM      │  │  Kernel API  │                        │
│  └──────────────┘  └──────────────┘                        │
└─────────────────────────────────────────────────────────────┘
```

## Module Descriptions

### Core Modules

#### 1. Configuration Manager (`config.h/c`)

**Responsibility**: Centralized configuration management

**Functions**:
- Load configuration from file
- Provide default values
- Expose configuration getters
- Validate configuration values

**Dependencies**: None (core module)

**Data Structures**:
```c
typedef struct {
    char backup_root[MAX_PATH_LENGTH];
    char log_dir[MAX_PATH_LENGTH];
    char registry_db[MAX_PATH_LENGTH];
    /* ... */
} app_config_t;
```

**Key APIs**:
- `config_init()` - Initialize configuration
- `config_load()` - Load from file
- `config_get_*()` - Getter functions

#### 2. Safe Execution (`safe_exec.h/c`)

**Responsibility**: Secure command execution and validation

**Functions**:
- Input validation
- Safe command execution (no shell)
- Safe string operations
- File system utilities

**Dependencies**: Configuration

**Key APIs**:
- `validate_*()` - Input validation functions
- `safe_exec()` - Execute command safely
- `safe_systemctl()` - Systemctl wrapper
- `safe_snprintf()` - Safe string formatting

**Security Features**:
- No shell interpretation (uses fork/execv)
- Comprehensive input validation
- Bounds checking on all operations

#### 3. Main Application (`main_new.c`)

**Responsibility**: Application entry point and event loop

**Functions**:
- Initialize subsystems
- Setup readline interface
- Main command loop
- Signal handling

**Dependencies**: All core modules

**Flow**:
```
1. Initialize configuration
2. Initialize database
3. Setup signal handlers
4. Setup readline
5. Enter command loop
   a. Read command
   b. Parse and validate
   c. Execute
   d. Handle result
6. Cleanup and exit
```

### Database Layer

#### Registry Database (`registry_new.h/c`)

**Responsibility**: Secure database operations

**Functions**:
- CRUD operations on all tables
- Parameterized queries only
- Transaction management
- Error handling

**Tables**:
1. `backup_points` - Backup point metadata
2. `nfs_shares` - NFS share configurations
3. `ssh_config` - SSH service configuration
4. `ssh_allowedhosts` - SSH access control

**Key APIs**:
- `registry_init()` - Initialize database
- `registry_insert_*()` - Insert operations
- `registry_update_*()` - Update operations
- `registry_delete_*()` - Delete operations
- `registry_list_*()` - Query operations

**Security Features**:
- ALL queries use parameterized statements
- No string concatenation for SQL
- Field name validation
- Proper error handling

### Command Modules

#### 1. Backup Point Management (`backup_point.c`)

**Responsibility**: BTRFS subvolume management

**Commands**:
- `create` - Create backup point
- `delete` - Delete backup point
- `rename` - Rename backup point
- `quota set` - Set quota
- `list` - List backup points

**Dependencies**: Safe Execution, Database, Configuration

**Workflow Example (create)**:
```
1. Validate backup point name
2. Check if already exists
3. Build safe command
4. Execute using safe_exec()
5. Record in database
6. Report success/failure
```

#### 2. Access Control (`access_control.c`)

**Responsibility**: Service and SSH management

**Commands**:
- SSH: `enable`, `disable`, `set port/protocol/address`
- FTP: `enable`, `disable`
- Web: `enable`, `disable`
- SSH allowed hosts: `add`, `delete`, `list`

**Dependencies**: Safe Execution, Database

**Workflow Example (ssh enable)**:
```
1. Call safe_systemctl("enable", "sshd")
2. Call safe_systemctl("start", "sshd")
3. Check return codes
4. Report success/failure
```

## Data Flow

### Command Execution Flow

```
User Input
    ↓
[Readline]
    ↓
[Command Parser]
    ↓
[Command Router] ──→ Find command in tree
    ↓
[Command Handler]
    ↓
[Input Validation] ──→ Reject invalid input
    ↓
[Business Logic]
    ↓
┌───┴────┬────────────┐
│        │            │
[DB]  [Exec]      [Other]
│        │            │
└───┬────┴────────────┘
    ↓
[Result Processing]
    ↓
[Output to User]
```

### Database Operation Flow

```
Command Module
    ↓
Registry API Call
    ↓
Parameter Validation
    ↓
Prepare Statement (parameterized)
    ↓
Bind Parameters (SQLite safe binding)
    ↓
Execute Statement
    ↓
Process Result
    ↓
Cleanup (finalize statement)
    ↓
Return to Caller
```

### Safe Execution Flow

```
Command Module
    ↓
Input Validation
    ↓
safe_exec() / safe_systemctl()
    ↓
Build argv array (no shell)
    ↓
fork()
    ↓
Child: execv() ──→ Kernel executes directly
    ↓
Parent: waitpid() ──→ Wait for completion
    ↓
Return exit code
```

## Security Architecture

### Defense in Depth

**Layer 1: Input Validation**
- Validate all user input before processing
- Whitelist approach (allow known-good)
- Type-specific validators

**Layer 2: Safe APIs**
- No direct system() or popen() calls
- All exec through safe_exec()
- All DB through parameterized queries

**Layer 3: Least Privilege**
- No setuid root
- Run with minimum required privileges
- Use sudo/capabilities for specific operations

**Layer 4: Error Handling**
- All errors caught and handled
- Fail securely (deny by default)
- No information leakage in errors

**Layer 5: Audit Logging**
- All operations logged
- User attribution
- Tamper-evident logs

### Trust Boundaries

```
┌─────────────────────────────────┐
│     Untrusted (User Input)      │
└────────────┬────────────────────┘
             │ VALIDATE
┌────────────┴────────────────────┐
│  Semi-Trusted (Validated Input) │
└────────────┬────────────────────┘
             │ SANITIZE & ESCAPE
┌────────────┴────────────────────┐
│    Trusted (Internal State)     │
└────────────┬────────────────────┘
             │ SAFE APIS
┌────────────┴────────────────────┐
│   External Systems (OS/DB)      │
└─────────────────────────────────┘
```

## Error Handling Strategy

### Error Codes
```c
#define SUCCESS 0
#define ERROR_GENERAL 1
#define ERROR_INVALID_ARGS 2
#define ERROR_AUTH_FAILED 3
#define ERROR_DB_ERROR 4
#define ERROR_SYSTEM_CALL 5
#define ERROR_MEMORY 6
#define ERROR_NOT_FOUND 7
#define ERROR_PERMISSION 8
```

### Error Flow
1. Detect error condition
2. Log error with context
3. Cleanup allocated resources
4. Return appropriate error code
5. Caller handles error gracefully
6. User receives helpful error message

## Performance Considerations

### Optimization Points

1. **Database Connection Pooling** (Future)
   - Single persistent connection
   - Prepared statement caching

2. **Command Completion**
   - Lazy loading of command tree
   - Cache frequently used completions

3. **Logging**
   - Asynchronous logging (future)
   - Configurable log levels

### Performance vs Security Trade-offs

| Feature | Performance Impact | Security Benefit | Decision |
|---------|-------------------|------------------|----------|
| Input Validation | ~microseconds | High | ✅ Keep |
| Parameterized Queries | Negligible | Critical | ✅ Keep |
| fork/exec vs system() | ~milliseconds | Critical | ✅ Keep |
| Bounds Checking | ~nanoseconds | High | ✅ Keep |

**Principle**: Security is more important than micro-optimizations in a CLI tool.

## Scalability

### Current Scale
- Small number of concurrent users (admin tool)
- Low frequency of operations
- Small database (<1MB typical)

### Future Considerations
- Multi-tenant support
- API interface (REST/gRPC)
- Distributed backup management
- High-availability configuration

## Testing Strategy

### Unit Tests
- Test individual functions in isolation
- Mock external dependencies
- Cover edge cases and error paths

### Integration Tests
- Test module interactions
- Real database operations
- Real file system operations (in sandbox)

### Security Tests
- Fuzzing inputs
- SQL injection attempts
- Command injection attempts
- Buffer overflow attempts

### Acceptance Tests
- End-to-end user scenarios
- Real-world workflows

## Deployment Architecture

```
┌─────────────────────────────────────┐
│         System Operator             │
└────────────┬────────────────────────┘
             │ sudo ./algocli
┌────────────┴────────────────────────┐
│         AlgoBackup CLI              │
│  ┌──────────────────────────────┐  │
│  │  Application Process         │  │
│  │  - User privileges           │  │
│  │  - Elevated via sudo         │  │
│  └──────────────────────────────┘  │
└────────┬────────────────┬───────────┘
         │                │
    ┌────┴────┐     ┌────┴─────┐
    │  SQLite │     │ Systemd  │
    │   DB    │     │ (System) │
    └─────────┘     └──────────┘
```

## Future Architecture Enhancements

### Phase 2: API Layer
- REST API for remote management
- WebSocket for real-time updates
- Authentication via JWT tokens

### Phase 3: Distributed Management
- Multi-node support
- Centralized configuration
- Cluster management

### Phase 4: Advanced Features
- Automated backup schedules
- Replication management
- Disaster recovery automation

## Conclusion

The refactored architecture provides:
- **Security**: Multiple layers of defense
- **Maintainability**: Clear separation of concerns
- **Extensibility**: Easy to add new commands
- **Testability**: Modular design enables comprehensive testing
- **Performance**: Optimized where it matters, secure everywhere

**Key Improvement**: The architecture prioritizes security without sacrificing functionality or performance.
