CXX = g++
BASE_CXXFLAGS = -std=c++17 -Wall -Wextra -I.
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
	@for test in $(TARGETS); do \
		echo "\n" \
		echo "Running $$test..."; \
		$$test; \
	done

clean:
	rm -rf $(BINDIR)

.PHONY: all test clean
