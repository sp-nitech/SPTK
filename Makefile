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

SOURCEDIR  := src
PYTHONDIR  := src/draw
BINDIR     := bin
BUILDDIR   := build
DOCDIR     := doc
INCLUDEDIR := include
LIBDIR     := lib

JOBS       := 4


all: build

build:
	mkdir -p $(BUILDDIR)
	cd $(BUILDDIR); cmake .. -DCMAKE_INSTALL_PREFIX=..
	cd $(BUILDDIR); make -j $(JOBS) install

doc:
	@if [ ! -f ./tools/venv/bin/activate ]; then \
		echo "Please prepare a Python environment via:"; \
		echo ""; \
		echo "  cd tools; make venv_dev"; \
		echo ""; \
		exit 1; \
	fi
	@if [ ! -x ./tools/doxygen/build/bin/doxygen ]; then \
		echo "Please install doxygen via:"; \
		echo ""; \
		echo "  cd tools; make doxygen.done"; \
		echo ""; \
		exit 1; \
	fi
	cd $(DOCDIR); ../tools/doxygen/build/bin/doxygen
	. ./tools/venv/bin/activate; cd $(DOCDIR); make html

doc-clean:
	rm -rf $(DOCDIR)/xml
	@if [ -f ./tools/venv/bin/activate ]; then \
		. ./tools/venv/bin/activate; cd $(DOCDIR); make clean; \
	fi

format: format-sh format-py format-cc

format-sh:
	@if [ ! -x ./tools/shellcheck/shellcheck ]; then \
		echo "Please install shellcheck via:"; \
		echo ""; \
		echo "  cd tools; make shellcheck.done"; \
		echo ""; \
		exit 1; \
	fi
	./tools/shellcheck/shellcheck egs/*/*/run.sh
	./tools/shellcheck/shellcheck -x test/*.bats

format-py:
	@if [ ! -x ./tools/venv/bin/black ] || [ ! -x ./tools/venv/bin/isort ] || [ ! -x ./tools/venv/bin/flake8 ]; then \
		echo "Please install black, isort, and flake8 via:"; \
		echo ""; \
		echo "  cd tools; make venv_dev"; \
		echo ""; \
		exit 1; \
	fi
	./tools/venv/bin/black $(PYTHONDIR)
	./tools/venv/bin/isort $(PYTHONDIR) --sl --fss --sort-order native --project sptk
	./tools/venv/bin/flake8 $(PYTHONDIR)

format-cc:
	@if [ ! -x ./tools/venv/bin/clang-format ] || [ ! -x ./tools/venv/bin/cpplint ]; then \
		echo "Please install clang-format and cpplint via:"; \
		echo ""; \
		echo "  cd tools; make venv_dev"; \
		echo ""; \
		exit 1; \
	fi
	./tools/venv/bin/clang-format -i $(wildcard $(SOURCEDIR)/*/*.cc)
	./tools/venv/bin/clang-format -i $(wildcard $(INCLUDEDIR)/SPTK/*/*.h)
	./tools/venv/bin/cpplint --filter=-readability/streams $(wildcard $(SOURCEDIR)/*/*.cc)
	./tools/venv/bin/cpplint --filter=-readability/streams,-build/include_subdir \
		--root=$(abspath $(INCLUDEDIR)) $(wildcard $(INCLUDEDIR)/SPTK/*/*.h)

test:
	@if [ ! -x ./tools/bats/bin/bats ]; then \
		echo "Please install bats via:"; \
		echo ""; \
		echo "  cd tools; make bats.done"; \
		echo ""; \
		exit 1; \
	fi
	./tools/bats/bin/bats --jobs $(JOBS) --no-parallelize-within-files test

test-clean:
	rm -rf test_*

clean: doc-clean test-clean
	rm -rf $(BUILDDIR) $(LIBDIR) $(BINDIR)

.PHONY: all build doc doc-clean format test test-clean clean
