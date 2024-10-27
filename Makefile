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


all: release

release:
	mkdir -p $(BUILDDIR)
	cd $(BUILDDIR) && cmake .. -DCMAKE_INSTALL_PREFIX=.. -DCMAKE_BUILD_TYPE=Release
	cd $(BUILDDIR) && make -j $(JOBS) install

debug:
	mkdir -p $(BUILDDIR)
	cd $(BUILDDIR) && cmake .. -DCMAKE_INSTALL_PREFIX=.. -DCMAKE_BUILD_TYPE=Debug
	cd $(BUILDDIR) && make -j $(JOBS) install

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
	cd $(DOCDIR) && ../tools/doxygen/build/bin/doxygen
	. ./tools/venv/bin/activate && cd $(DOCDIR) && make html

doc-clean:
	rm -rf $(DOCDIR)/xml
	@if [ -f ./tools/venv/bin/activate ]; then \
		. ./tools/venv/bin/activate && cd $(DOCDIR) && make clean; \
	fi

example:
	@for task in egs/*; do \
		for kind in $$task/*; do \
			echo "Checking example in $$kind..."; \
			cd $$kind && ./run.sh && cd - > /dev/null; \
		done; \
	done

format: format-sh format-py format-cc format-misc

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
	@if [ ! -x ./tools/shfmt/shfmt ]; then \
		echo "Please install shfmt via:"; \
		echo ""; \
		echo "  cd tools; make shfmt.done"; \
		echo ""; \
		exit 1; \
	fi
	./tools/shfmt/shfmt -i 4 -ci -sr -kp -w egs/*/*/run.sh test/*.bats

format-py:
	@if [ ! -x ./tools/venv/bin/black ] || [ ! -x ./tools/venv/bin/isort ] || [ ! -x ./tools/venv/bin/flake8 ]; then \
		echo "Please install black, isort, and flake8 via:"; \
		echo ""; \
		echo "  cd tools; make venv_dev"; \
		echo ""; \
		exit 1; \
	fi
	./tools/venv/bin/black $(PYTHONDIR) $(DOCDIR)
	./tools/venv/bin/isort $(PYTHONDIR) $(DOCDIR) --sl --fss --sort-order native --project sptk
	./tools/venv/bin/flake8 $(PYTHONDIR) $(DOCDIR)

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
	@if [ ! -x ./tools/cppcheck/cppcheck ]; then \
		echo "Please install cppcheck via:"; \
		echo ""; \
		echo "  cd tools; make cppcheck.done"; \
		echo ""; \
		exit 1; \
	fi
	mkdir -p $(BUILDDIR)/cppcheck
	./tools/cppcheck/cppcheck -j $(JOBS) --enable=all --check-level=exhaustive --std=c++11 \
		--suppress=checkersReport --suppress=constParameterPointer --suppress=missingIncludeSystem \
		--suppress=unmatchedSuppression --suppress=unusedFunction --suppress=useStlAlgorithm \
		--suppress=variableScope --inline-suppr --error-exitcode=1 \
		--cppcheck-build-dir=$(BUILDDIR)/cppcheck --checkers-report=$(BUILDDIR)/cppcheck/report.txt \
		-Iinclude -Ithird_party src

format-misc:
	@if [ ! -x ./tools/venv/bin/cmake-format ] || [ ! -x ./tools/venv/bin/mdformat ] || [ ! -x ./tools/venv/bin/docstrfmt ]; then \
		echo "Please install cmake-format, mdformat, and docstrfmt via:"; \
		echo ""; \
		echo "  cd tools; make venv_dev"; \
		echo ""; \
		exit 1; \
	fi
	./tools/venv/bin/cmake-format -i CMakeLists.txt
	./tools/venv/bin/mdformat *.md
	cd $(DOCDIR) && ../tools/venv/bin/docstrfmt .

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

tool:
	cd tools && make

clean: doc-clean test-clean
	rm -rf $(BUILDDIR) $(LIBDIR) $(BINDIR)

.PHONY: all build doc doc-clean example format test test-clean tool clean
