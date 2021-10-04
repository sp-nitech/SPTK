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

SOURCEDIR      = src
MAINSOURCEDIR  = $(SOURCEDIR)/main
BUILDDIR       = build
INCLUDEDIR     = include
LIBDIR         = lib
BINDIR         = bin
DOCDIR         = doc
THIRDPARTYDIR  = third_party
THIRDPARTYDIRS = $(wildcard $(THIRDPARTYDIR)/*)

TARGET         = $(LIBDIR)/libsptk.a
MAINSOURCES    = $(wildcard $(MAINSOURCEDIR)/*.cc)
SOURCES        = $(filter-out $(MAINSOURCES), $(wildcard $(SOURCEDIR)/*/*.cc))
OBJECTS        = $(patsubst $(SOURCEDIR)/%.cc, $(BUILDDIR)/%.o, $(SOURCES))
BINARIES       = $(patsubst $(MAINSOURCEDIR)/%.cc, $(BINDIR)/%, $(MAINSOURCES))

MAKE           = make
CXX            = g++
AR             = ar
CXXFLAGS       = -Wall -O2 -g -std=c++11
LIBFLAGS       = -lm -lstdc++
INCLUDE        = -I $(INCLUDEDIR) -I $(THIRDPARTYDIR)

all: $(THIRDPARTYDIRS) $(TARGET) $(BINARIES)

$(BINARIES): $(BINDIR)/%: $(MAINSOURCEDIR)/%.cc
	mkdir -p $(BINDIR)
	$(CXX) $(LIBFLAGS) $(CXXFLAGS) $(INCLUDE) $< $(TARGET) -o $@

$(TARGET): $(OBJECTS)
	mkdir -p $(LIBDIR)
	$(AR) cru $(TARGET) $(OBJECTS) $(wildcard $(THIRDPARTYDIR)/*/build/*/*.o)

$(OBJECTS): $(BUILDDIR)/%.o: $(SOURCEDIR)/%.cc
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ -c $<

$(THIRDPARTYDIRS):
	$(MAKE) -C $@

doc:
	cd $(DOCDIR); ../tools/doxygen/build/bin/doxygen
	. ./tools/venv/bin/activate; cd $(DOCDIR); make html

doc-clean:
	. ./tools/venv/bin/activate; cd $(DOCDIR); make clean

format:
	clang-format -i $(wildcard $(SOURCEDIR)/*/*.cc)
	clang-format -i	$(wildcard $(INCLUDEDIR)/SPTK/*/*.h)
	./tools/cpplint/cpplint.py --filter=-readability/streams $(wildcard $(SOURCEDIR)/*/*.cc)
	./tools/cpplint/cpplint.py --filter=-readability/streams,-build/include_subdir \
		--root=$(abspath $(INCLUDEDIR)) $(wildcard $(INCLUDEDIR)/SPTK/*/*.h)

shellcheck:
	./tools/shellcheck/shellcheck egs/*/*/run.sh

test:
	./tools/bats/bin/bats test

clean: doc-clean
	for dir in $(THIRDPARTYDIRS); do \
		$(MAKE) clean -C $$dir; \
	done
	rm -rf $(BUILDDIR) $(LIBDIR) $(BINDIR) $(DOCDIR)/xml

.PHONY: all $(THIRDPARTYDIRS) doc doc-clean format shellcheck test clean
