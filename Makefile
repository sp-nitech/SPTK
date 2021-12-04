# ------------------------------------------------------------------------ #
# Copyright 2021 SPTK Working Group                                        #
#                                                                          #
# Licensed under the Apache License, Version 2.0 (the "License");          #
# you may not use this file except in compliance with the License.         #
# You may obtain a copy of the License at                                  #
#                                                                          #
#     http://www.apache.org/licenses/LICENSE-2.0                           #
#                                                                          #
# Unless required by applicable law or agreed to in writing, software      #
# distributed under the License is distributed on an "AS IS" BASIS,        #
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. #
# See the License for the specific language governing permissions and      #
# limitations under the License.                                           #
# ------------------------------------------------------------------------ #

SOURCEDIR      := src
MAINSOURCEDIR  := $(SOURCEDIR)/main
PYTHONDIR      := $(SOURCEDIR)/draw
BINDIR         := bin
BUILDDIR       := build
DOCDIR         := doc
INCLUDEDIR     := include
LIBDIR         := lib
THIRDPARTYDIR  := third_party
THIRDPARTYDIRS := $(wildcard $(THIRDPARTYDIR)/*)

LIBRARY        := $(LIBDIR)/libsptk.a
MAINSOURCES    := $(wildcard $(MAINSOURCEDIR)/*.cc)
SOURCES        := $(filter-out $(MAINSOURCES), $(wildcard $(SOURCEDIR)/*/*.cc))
OBJECTS        := $(patsubst $(SOURCEDIR)/%.cc, $(BUILDDIR)/%.o, $(SOURCES))
BINARIES       := $(patsubst $(MAINSOURCEDIR)/%.cc, $(BINDIR)/%, $(MAINSOURCES))
PYTHONS        := $(wildcard $(PYTHONDIR)/*.py)
PYTHONLINKS    := $(patsubst $(PYTHONDIR)/%.py, $(BINDIR)/%, $(PYTHONS))

MAKE           := make
CXX            := g++
AR             := ar
CXXFLAGS       := -Wall -O2 -g -std=c++11
INCLUDE        := -I $(INCLUDEDIR) -I $(THIRDPARTYDIR)

JOBS           := 4


all: $(BINARIES) $(PYTHONLINKS)

$(BINARIES): $(BINDIR)/%: $(MAINSOURCEDIR)/%.cc $(LIBRARY)
	@if [ ! -d $(BINDIR) ]; then \
		mkdir -p $(BINDIR); \
	fi
	$(CXX) $(CXXFLAGS) $(INCLUDE) $< $(THIRDPARTYDIR)/Getopt/getoptwin.cc $(LIBRARY) -o $@

$(LIBRARY): $(OBJECTS)
	@if [ ! -d $(LIBDIR) ]; then \
		mkdir -p $(LIBDIR); \
	fi
	for dir in $(THIRDPARTYDIRS); do \
		$(MAKE) -C $$dir; \
	done
	$(AR) cru $(LIBRARY) $(OBJECTS) $(THIRDPARTYDIR)/*/build/*/*.o

$(OBJECTS): $(BUILDDIR)/%.o: $(SOURCEDIR)/%.cc
	@if [ ! -d $(dir $@) ]; then \
		mkdir -p $(dir $@); \
	fi
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ -c $<

$(PYTHONLINKS):
	@if [ ! -d $(BINDIR) ]; then \
		mkdir -p $(BINDIR); \
	fi
	ln -snf ../$(PYTHONDIR)/$(notdir $@).py $@

doc:
	cd $(DOCDIR); ../tools/doxygen/build/bin/doxygen
	. ./tools/venv/bin/activate; cd $(DOCDIR); make html

doc-clean:
	rm -rf $(DOCDIR)/xml
	. ./tools/venv/bin/activate; cd $(DOCDIR); make clean

format:
	# Bash
	./tools/shellcheck/shellcheck egs/*/*/run.sh
	./tools/shellcheck/shellcheck -x test/*.bats

	# Python
	./tools/venv/bin/black $(PYTHONDIR)
	./tools/venv/bin/isort $(PYTHONDIR) --sl --fss --sort-order native --project sptk
	./tools/venv/bin/flake8 $(PYTHONDIR)

	# C++
	clang-format -i $(wildcard $(SOURCEDIR)/*/*.cc)
	clang-format -i	$(wildcard $(INCLUDEDIR)/SPTK/*/*.h)
	./tools/cpplint/cpplint.py --filter=-readability/streams $(wildcard $(SOURCEDIR)/*/*.cc)
	./tools/cpplint/cpplint.py --filter=-readability/streams,-build/include_subdir \
		--root=$(abspath $(INCLUDEDIR)) $(wildcard $(INCLUDEDIR)/SPTK/*/*.h)

test:
	./tools/bats/bin/bats --jobs $(JOBS) --no-parallelize-within-files test

clean: doc-clean
	for dir in $(THIRDPARTYDIRS); do \
		$(MAKE) clean -C $$dir; \
	done
	rm -rf $(BUILDDIR) $(LIBDIR) $(BINDIR)

.PHONY: all doc doc-clean format test clean
