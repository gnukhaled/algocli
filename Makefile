# AlgoBackup CLI - Modernized Makefile
# Refactored version with security best practices

# Project information
PROJECT = algocli
VERSION = 0.2.0
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin
SYSCONFDIR = /etc/algobackup
DATADIR = $(PREFIX)/share/algobackup
LOGDIR = /var/log/algobackup
DBDIR = /var/lib/algobackup

# Compiler and flags
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Werror -Wpedantic \
         -Wformat=2 -Wformat-security -Werror=format-security \
         -fstack-protector-strong -fPIE \
         -D_FORTIFY_SOURCE=2 \
         -O2 -g

# Additional security flags
CFLAGS_SECURITY = -Wl,-z,relro,-z,now -pie

# Include directories
INCLUDES = -Iinclude/core -Iinclude/database -Iinclude/commands -Iinclude/logging

# Libraries
# Note: PAM and smartcols removed for now (not used in refactored core)
LIBS = -lreadline -lsqlite3

# Linker flags
LDFLAGS = $(CFLAGS_SECURITY)

# Source files (new refactored version)
CORE_SRCS = src/core/config.c \
            src/core/safe_exec.c \
            src/core/command_dispatcher.c \
            src/core/commands.c \
            src/core/main_new.c

DB_SRCS = src/database/registry_new.c

CMD_SRCS = src/commands/backup_point.c \
           src/commands/access_control.c

# All source files
SRCS_NEW = $(CORE_SRCS) $(DB_SRCS) $(CMD_SRCS)

# Object files (new)
OBJS_NEW = $(SRCS_NEW:.c=.o)

# Old source files (original codebase)
SRCS_OLD = main.c algocli.c registry.c algo-log.c \
           cmd-bp.c cmd-accessctrl.c cmd-fs.c cmd-log.c \
           cmd-nfs.c cmd-system.c

# Object files (old)
OBJS_OLD = $(SRCS_OLD:.c=.o)

# Build targets
.PHONY: all clean install uninstall test scan help old new

# Default target builds new version
all: new

# Build new (refactored) version
new: $(PROJECT)-new

$(PROJECT)-new: $(OBJS_NEW)
	@echo "Linking $(PROJECT)-new..."
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
	@echo "Build complete: $(PROJECT)-new"
	@echo ""
	@echo "⚠️  SECURITY NOTE:"
	@echo "   This version does NOT use setuid."
	@echo "   Run with sudo or appropriate capabilities."
	@echo ""

# Build old (original) version - FOR COMPARISON ONLY
old: $(PROJECT)-old

$(PROJECT)-old: $(OBJS_OLD)
	@echo "⚠️  Building INSECURE original version for comparison only!"
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
	@echo "⚠️  DO NOT USE IN PRODUCTION - Contains known vulnerabilities!"

# Compile source files
%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Installation
install: $(PROJECT)-new
	@echo "Installing AlgoBackup CLI..."
	install -d $(DESTDIR)$(BINDIR)
	install -d $(DESTDIR)$(SYSCONFDIR)
	install -d $(DESTDIR)$(DATADIR)
	install -d $(DESTDIR)$(LOGDIR)
	install -d $(DESTDIR)$(DBDIR)
	install -m 0755 $(PROJECT)-new $(DESTDIR)$(BINDIR)/$(PROJECT)
	install -m 0644 config/templates/registry.ddl $(DESTDIR)$(DATADIR)/
	@echo ""
	@echo "Installation complete!"
	@echo ""
	@echo "Configuration directory: $(SYSCONFDIR)"
	@echo "Data directory:          $(DATADIR)"
	@echo "Log directory:           $(LOGDIR)"
	@echo "Database directory:      $(DBDIR)"
	@echo ""
	@echo "Next steps:"
	@echo "  1. Initialize database: sudo $(BINDIR)/$(PROJECT) init"
	@echo "  2. Run: sudo $(BINDIR)/$(PROJECT)"

# Uninstallation
uninstall:
	@echo "Uninstalling AlgoBackup CLI..."
	rm -f $(DESTDIR)$(BINDIR)/$(PROJECT)
	rm -rf $(DESTDIR)$(DATADIR)
	@echo "Note: Configuration, logs, and database preserved"
	@echo "      Remove manually if needed:"
	@echo "      - $(SYSCONFDIR)"
	@echo "      - $(LOGDIR)"
	@echo "      - $(DBDIR)"

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(OBJS_NEW) $(OBJS_OLD)
	rm -f $(PROJECT)-new $(PROJECT)-old
	rm -f src/core/*.o src/database/*.o src/commands/*.o src/logging/*.o
	rm -f *.o
	@echo "Clean complete."

# Deep clean (including generated files)
distclean: clean
	@echo "Deep cleaning..."
	rm -f *~ src/*~ src/core/*~ src/database/*~ src/commands/*~
	rm -f include/*~ include/core/*~ include/database/*~ include/commands/*~
	@echo "Deep clean complete."

# Security scanning
scan: $(SRCS_NEW)
	@echo "Running security scans..."
	@echo ""
	@echo "=== Static Analysis ==="
	@command -v cppcheck >/dev/null 2>&1 && \
		cppcheck --enable=all --inconclusive --std=c99 $(SRCS_NEW) || \
		echo "⚠️  cppcheck not installed (optional)"
	@echo ""
	@echo "=== Security Linting ==="
	@command -v flawfinder >/dev/null 2>&1 && \
		flawfinder $(SRCS_NEW) || \
		echo "⚠️  flawfinder not installed (optional)"
	@echo ""
	@echo "Scan complete. Review output above."

# Format code
format:
	@echo "Formatting code..."
	@command -v clang-format >/dev/null 2>&1 && \
		find src include -name "*.c" -o -name "*.h" | xargs clang-format -i || \
		echo "⚠️  clang-format not installed (optional)"

# Generate documentation
docs:
	@echo "Generating documentation..."
	@command -v doxygen >/dev/null 2>&1 && \
		doxygen Doxyfile || \
		echo "⚠️  doxygen not installed (optional)"

# Run tests (placeholder - tests need to be implemented)
test:
	@echo "Running tests..."
	@echo "⚠️  Test suite not yet implemented"
	@echo "   TODO: Implement unit tests"
	@echo "   TODO: Implement integration tests"
	@echo "   TODO: Implement security tests"

# Help target
help:
	@echo "AlgoBackup CLI - Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all         - Build new (refactored) version [default]"
	@echo "  new         - Build new (refactored) version"
	@echo "  old         - Build old (original) version [INSECURE - comparison only]"
	@echo "  install     - Install to system (requires sudo)"
	@echo "  uninstall   - Remove from system (requires sudo)"
	@echo "  clean       - Remove build artifacts"
	@echo "  distclean   - Deep clean including generated files"
	@echo "  scan        - Run security scanners (cppcheck, flawfinder)"
	@echo "  format      - Format code with clang-format"
	@echo "  docs        - Generate documentation with doxygen"
	@echo "  test        - Run test suite [TODO]"
	@echo "  help        - Show this help message"
	@echo ""
	@echo "Variables:"
	@echo "  PREFIX      - Installation prefix (default: /usr/local)"
	@echo "  DESTDIR     - Staging directory for packaging"
	@echo "  CC          - C compiler (default: gcc)"
	@echo ""
	@echo "Examples:"
	@echo "  make                    # Build new version"
	@echo "  make scan               # Run security scans"
	@echo "  sudo make install       # Install to system"
	@echo "  make PREFIX=/opt/algo   # Custom install location"

# Dependency generation (for incremental builds)
.depend: $(SRCS_NEW)
	@echo "Generating dependencies..."
	@$(CC) $(CFLAGS) $(INCLUDES) -MM $(SRCS_NEW) > .depend

-include .depend

# Prevent deletion of intermediate files
.PRECIOUS: %.o

# Mark phony targets
.PHONY: all new old clean distclean install uninstall test scan format docs help
