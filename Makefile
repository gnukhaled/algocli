# Makefile to build the AlgoBackup CLI shell.
#
# Layout:
#   make           — build ./algocli
#   make clean     — remove build artifacts
#   make analyze   — run cppcheck and gcc -fanalyzer
#   make BUILD=asan — debug build with AddressSanitizer + UBSan
#
# Phase 0 baseline: builds clean under gcc-15 with -Wall -Wextra -Wpedantic
# plus the hardening flags below. -Werror is intentionally NOT enabled
# yet — Phase 1B will rewrite registry.c and remove the last #pragma
# diagnostic suppressions, after which -Werror becomes feasible.

CC      ?= gcc
PREFIX  ?= /usr/local
BUILD   ?= release

WARN_FLAGS = -Wall -Wextra -Wpedantic -Wstrict-prototypes \
             -Wformat=2 -Wformat-security -Wnull-dereference \
             -Wshadow -Wpointer-arith -Wcast-qual

HARDEN_FLAGS = -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fPIE
HARDEN_LDFLAGS = -pie -Wl,-z,relro,-z,now

CFLAGS  ?= -O2 -g
# Mandatory flags. `override` ensures these survive even when the user
# supplies CFLAGS on the command line (e.g. `make CFLAGS+=-DBACKUP_ROOT=…`).
override CFLAGS += -std=c11 -D_GNU_SOURCE $(WARN_FLAGS) $(HARDEN_FLAGS) -MMD -MP \
                   -DALGOCLI_VERSION='"$(VERSION)"' $(EXTRA_CFLAGS)

# Version string. If we're inside a git checkout, prefer `git describe`;
# otherwise fall back to a static placeholder. Override on the command
# line for tagged release builds: `make VERSION=1.2.3 install`.
VERSION ?= $(shell git -C $(CURDIR) describe --tags --dirty --always 2>/dev/null || echo dev)

LDFLAGS += $(HARDEN_LDFLAGS)
LDLIBS  := -lreadline -lpam -lpam_misc -lsqlite3 -lsmartcols

ifeq ($(BUILD),asan)
  # _FORTIFY_SOURCE wants -O>0; ASan wants -O0 for sharp traces. Drop
  # the fortify define and keep -O1 so glibc doesn't warn.
  CFLAGS  := $(filter-out -O2 -D_FORTIFY_SOURCE=2, $(CFLAGS))
  CFLAGS  += -O1 -fsanitize=address,undefined -fno-omit-frame-pointer
  LDFLAGS += -fsanitize=address,undefined
endif

TARGET := algocli

# Phase 0: only build the modules that are actually populated. The
# eight empty stub files (cmd-cifs/config/disk/net/repl/snapshot/
# syshealth/user) are excluded — every func_* stub for those areas
# already lives in algocli.c, so the empty .c files contribute nothing
# and only trigger ISO-C empty-translation-unit warnings. They re-enter
# the build in Phase 4 when the stubs become real implementations.
SRCS := main.c algocli.c algocli-defs.c algo-log.c registry.c \
        validate.c proc.c privilege.c auth.c \
        cmd-system.c cmd-fs.c cmd-bp.c cmd-accessctrl.c \
        cmd-log.c cmd-nfs.c
OBJS := $(SRCS:.c=.o)
DEPS := $(OBJS:.o=.d)

# Test programs — compiled and run by `make check`. Each test is a
# self-contained C file under tests/ that links the units it exercises
# directly (no shared test runtime, no external framework).
TEST_BINS := tests/test_validate tests/test_registry

.PHONY: all clean analyze install check ci

all: $(TARGET)

# `make ci` — full pre-commit gate. Builds clean, static-analyzes,
# tests. Intended to be the one command an author runs before
# proposing a change.
ci:
	$(MAKE) clean
	$(MAKE) EXTRA_CFLAGS=-Werror
	$(MAKE) analyze
	$(MAKE) check
	@echo
	@echo "  ci: all green"

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Static analysis target. Non-blocking — it only reports findings.
# Phase 5 wires this into a `make ci` target with a baseline.
analyze:
	@if command -v cppcheck >/dev/null 2>&1; then \
	  cppcheck --enable=warning,style,performance,portability \
	           --inconclusive --quiet \
	           --suppress=missingIncludeSystem $(SRCS); \
	else \
	  echo "cppcheck not installed — skipping"; \
	fi
	@for f in $(SRCS); do \
	  echo "  ANALYZE  $$f"; \
	  $(CC) $(CFLAGS) -fanalyzer -fsyntax-only $$f || exit 1; \
	done

check: $(TEST_BINS) $(TARGET)
	@for t in $(TEST_BINS); do \
	  echo "  TEST     $$t"; \
	  ./$$t || exit 1; \
	done
	@echo "  TEST     tests/test_dispatch.sh"
	@./tests/test_dispatch.sh

tests/test_validate: tests/test_validate.c validate.c validate.h
	$(CC) $(CFLAGS) -I. tests/test_validate.c validate.c -o $@

tests/test_registry: tests/test_registry.c registry.c registry.h
	$(CC) $(CFLAGS) -I. tests/test_registry.c registry.c -lsqlite3 -o $@

# Install the binary, the man page, and the PAM service file. The
# binary goes in mode 4755 (setuid bit on top of 0755); the PAM file
# is required for any state-changing command to authenticate. Override
# DESTDIR for staged installs (packaging) and PREFIX for layout choice.
#
# Ownership is only forced when running as root — for staged builds
# under DESTDIR the packager (rpm/dpkg/fakeroot) handles ownership at
# deploy time. INSTALL_OWNER and INSTALL_GROUP can be overridden.
INSTALL_OWNER ?= root
INSTALL_GROUP ?= wheel
ifeq ($(shell id -u),0)
  INSTALL_OWN_FLAGS = -o $(INSTALL_OWNER) -g $(INSTALL_GROUP)
else
  INSTALL_OWN_FLAGS =
endif

install: $(TARGET) algocli.1 templates/pam.d.algocli
	install -d $(DESTDIR)$(PREFIX)/sbin
	install -d $(DESTDIR)$(PREFIX)/share/man/man1
	install -d $(DESTDIR)/etc/pam.d
	install -m 4755 $(INSTALL_OWN_FLAGS) $(TARGET) $(DESTDIR)$(PREFIX)/sbin/$(TARGET)
	install -m 0644 algocli.1 $(DESTDIR)$(PREFIX)/share/man/man1/algocli.1
	@if [ -e $(DESTDIR)/etc/pam.d/algocli ]; then \
	  echo "  preserving existing $(DESTDIR)/etc/pam.d/algocli"; \
	else \
	  install -m 0644 templates/pam.d.algocli $(DESTDIR)/etc/pam.d/algocli; \
	fi
	@echo
	@echo "  installed:"
	@echo "    $(DESTDIR)$(PREFIX)/sbin/$(TARGET)         (mode 4755)"
	@echo "    $(DESTDIR)$(PREFIX)/share/man/man1/algocli.1"
	@echo "    $(DESTDIR)/etc/pam.d/algocli"
	@if [ "$$(id -u)" != "0" ]; then \
	  echo; \
	  echo "  NOTE: ownership was not changed because make ran as a"; \
	  echo "        non-root user. For an active deployment, ensure the"; \
	  echo "        binary is owned by root:wheel."; \
	fi

clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS) $(TEST_BINS)

-include $(DEPS)
