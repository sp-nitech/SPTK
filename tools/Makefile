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

PYTHON_VERSION     := 3.8
SHELLCHECK_VERSION := 0.7.2
JOBS               := 4


all: env test_env bats.done cpplint.done doxygen.done shellcheck.done sptk.done

env:
	test -d venv || virtualenv -p python$(PYTHON_VERSION) venv
	. venv/bin/activate; pip install -r requirements.txt --upgrade
	touch venv/bin/activate

test_env:
	test -d venv || virtualenv -p python$(PYTHON_VERSION) venv
	. venv/bin/activate; pip install -r test_requirements.txt --upgrade
	touch venv/bin/activate

bats.done:
	rm -rf bats
	git clone https://github.com/bats-core/bats-core.git bats --depth 1
	touch bats.done

cpplint.done:
	rm -rf cpplint
	git clone https://github.com/cpplint/cpplint.git --depth 1
	touch cpplint.done

doxygen.done:
	rm -rf doxygen
	git clone https://github.com/doxygen/doxygen.git -b Release_1_9_1 --depth 1
	cd doxygen; mkdir -p build; cd build; cmake -G "Unix Makefiles" ..; make
	touch doxygen.done

shellcheck.done:
	rm -rf shellcheck
	wget https://github.com/koalaman/shellcheck/releases/download/v$(SHELLCHECK_VERSION)/shellcheck-v$(SHELLCHECK_VERSION).linux.x86_64.tar.xz
	tar -xf shellcheck-v$(SHELLCHECK_VERSION).linux.x86_64.tar.xz
	rm -f shellcheck-v$(SHELLCHECK_VERSION).linux.x86_64.tar.xz
	mv shellcheck-v$(SHELLCHECK_VERSION) shellcheck
	touch shellcheck.done

sptk.done:
	rm -rf sptk
	export GIT_SSL_NO_VERIFY=1; git clone https://git.code.sf.net/p/sp-tk/SPTK sptk
	cd sptk/src; aclocal; automake -a -c; autoconf; \
	./configure --enable-double --prefix=$(CURDIR)/sptk; make -j $(JOBS); make install
	touch sptk.done

clean:
	rm -rf venv bats cpplint doxygen shellcheck sptk *.done

.PHONY: all clean
