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

SOURCEDIR  := .
BUILDDIR   := build
INCLUDEDIR := generic

SOURCES    := $(wildcard $(SOURCEDIR)/*/*.cc)
OBJECTS    := $(patsubst $(SOURCEDIR)/%.cc, $(BUILDDIR)/%.o, $(SOURCES))

CXX        := g++
AR         := ar
CXXFLAGS   := -Wall -O2 -g -std=c++11
INCLUDE    := -I $(INCLUDEDIR) -I ../../include

all: $(OBJECTS)

$(OBJECTS): $(BUILDDIR)/%.o: $(SOURCEDIR)/%.cc
	@if [ ! -d $(dir $@) ]; then \
		mkdir -p $(dir $@); \
	fi
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ -c $<

clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean
