CXX = g++
BASE_CXXFLAGS = -std=c++20 -Wall -Wextra -I.
SANITIZER_FLAGS = -fsanitize=address,undefined -fno-omit-frame-pointer -g
CXXFLAGS = $(BASE_CXXFLAGS) $(SANITIZER_FLAGS)

SRCDIR = tests
BINDIR = bin

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
TARGETS = $(patsubst $(SRCDIR)/%.cpp,$(BINDIR)/%,$(SOURCES))

all: $(TARGETS)

$(BINDIR)/%: $(SRCDIR)/%.cpp
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $< -o $@

test: $(TARGETS)
	@set -e; \
	for test in $(TARGETS); do \
		echo "\nRunning $$test..."; \
		if ! $$test; then \
			echo "\nTest failed: $$test"; \
			exit 1; \
		fi; \
	done; \
	echo "\nAll tests are passed."

clean:
	rm -rf $(BINDIR)

.PHONY: all test clean
