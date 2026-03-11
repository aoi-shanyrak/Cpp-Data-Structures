CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I.

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
		echo "Running $$test..."; \
		$$test; \
	done

clean:
	rm -rf $(BINDIR)

.PHONY: all test clean
