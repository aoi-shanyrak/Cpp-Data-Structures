CXX = g++
BASE_CXXFLAGS = -std=c++20 -Wall -Wextra -I.
SANITIZER_FLAGS = -fsanitize=address,undefined -fno-omit-frame-pointer -g
TEST_DEFINE ?=
CXXFLAGS = $(BASE_CXXFLAGS) $(SANITIZER_FLAGS) $(TEST_DEFINE)

SRCDIR = tests
BINDIR = bin
BINDIR_DEEP = $(BINDIR)/deep

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
TARGETS = $(patsubst $(SRCDIR)/%.cpp,$(BINDIR)/%,$(SOURCES))
TARGETS_DEEP = $(patsubst $(SRCDIR)/%.cpp,$(BINDIR_DEEP)/%,$(SOURCES))


all: $(TARGETS)

$(BINDIR)/%: $(SRCDIR)/%.cpp
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BINDIR_DEEP)/%: $(SRCDIR)/%.cpp
	@mkdir -p $(BINDIR_DEEP)
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

deeptest: TEST_DEFINE = -DDEEP_TEST
deeptest: $(TARGETS_DEEP)
	@set -e; \
	for test in $(TARGETS_DEEP); do \
		echo "\nRunning $$test..."; \
		if ! $$test; then \
			echo "\nTest failed: $$test"; \
			exit 1; \
		fi; \
	done; \
	echo "\nAll deep tests are passed."

clean:
	rm -rf $(BINDIR)

.PHONY: all test deeptest clean
