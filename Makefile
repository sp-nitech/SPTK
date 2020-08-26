# ----------------------------------------------------------------- #
#             The Speech Signal Processing Toolkit (SPTK)           #
#             developed by SPTK Working Group                       #
#             http://sp-tk.sourceforge.net/                         #
# ----------------------------------------------------------------- #
#                                                                   #
#  Copyright (c) 1984-2007  Tokyo Institute of Technology           #
#                           Interdisciplinary Graduate School of    #
#                           Science and Engineering                 #
#                                                                   #
#                1996-2020  Nagoya Institute of Technology          #
#                           Department of Computer Science          #
#                                                                   #
# All rights reserved.                                              #
#                                                                   #
# Redistribution and use in source and binary forms, with or        #
# without modification, are permitted provided that the following   #
# conditions are met:                                               #
#                                                                   #
# - Redistributions of source code must retain the above copyright  #
#   notice, this list of conditions and the following disclaimer.   #
# - Redistributions in binary form must reproduce the above         #
#   copyright notice, this list of conditions and the following     #
#   disclaimer in the documentation and/or other materials provided #
#   with the distribution.                                          #
# - Neither the name of the SPTK working group nor the names of its #
#   contributors may be used to endorse or promote products derived #
#   from this software without specific prior written permission.   #
#                                                                   #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            #
# CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       #
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          #
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          #
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS #
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          #
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   #
# TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     #
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON #
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   #
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    #
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           #
# POSSIBILITY OF SUCH DAMAGE.                                       #
# ----------------------------------------------------------------- #

SOURCEDIR      = src
MAINSOURCEDIR  = $(SOURCEDIR)/main
BUILDDIR       = build
INCLUDEDIR     = include
LIBDIR         = lib
BINDIR         = bin
THIRDPARTYDIR  = third_party
THIRDPARTYDIRS = $(wildcard $(THIRDPARTYDIR)/*)

TARGET        = $(LIBDIR)/libsptk.a
MAINSOURCES   = $(wildcard $(MAINSOURCEDIR)/*.cc)
SOURCES       = $(filter-out $(MAINSOURCES), $(wildcard $(SOURCEDIR)/*/*.cc))
OBJECTS       = $(patsubst $(SOURCEDIR)/%.cc, $(BUILDDIR)/%.o, $(SOURCES))
BINARIES      = $(patsubst $(MAINSOURCEDIR)/%.cc, $(BINDIR)/%, $(MAINSOURCES))

MAKE          = make
CXX           = g++
AR            = ar
CXXFLAGS      = -Wall -O2 -g -std=c++11
LIBFLAGS      = -lm -lstdc++
INCLUDE       = -I $(INCLUDEDIR) -I $(THIRDPARTYDIR)

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
	cd doc; ../tools/doxygen/build/bin/doxygen
	. ./tools/venv/bin/activate; cd doc; make html

format:
	clang-format -i $(wildcard $(SOURCEDIR)/*/*.cc)
	clang-format -i	$(wildcard $(INCLUDEDIR)/SPTK/*/*.h)

test:
	./tools/bats/bin/bats test

clean:
	for dir in $(THIRDPARTYDIRS); do \
		$(MAKE) clean -C $$dir; \
	done
	rm -rf $(BUILDDIR) $(LIBDIR) $(BINDIR)

.PHONY: all $(THIRDPARTYDIRS) doc format test clean
