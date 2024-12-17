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

PYTHON_VERSION     := 3.9
BATS_VERSION       := 1.11.0
CPPCHECK_VERSION   := 2.16.0
DOXYGEN_VERSION    := 1.9.1
SHELLCHECK_VERSION := 0.10.0
SHFMT_VERSION      := 3.10.0
JOBS               := 4


all: check doxygen.done

check: venv_dev bats.done cppcheck.done shellcheck.done shfmt.done sptk.done

venv:
	test -d venv || python$(PYTHON_VERSION) -m venv venv
	. ./venv/bin/activate && python -m pip install pip --upgrade
	. ./venv/bin/activate && python -m pip install -r requirements.txt --upgrade
	touch venv/bin/activate

venv_dev:
	test -d venv || python$(PYTHON_VERSION) -m venv venv
	. ./venv/bin/activate && python -m pip install pip --upgrade
	. ./venv/bin/activate && python -m pip install -r requirements.txt -r test_requirements.txt --upgrade
	touch venv/bin/activate

bats.done:
	rm -rf bats
	git clone https://github.com/bats-core/bats-core.git bats -b v$(BATS_VERSION) --single-branch --depth 1
	touch bats.done

cppcheck.done:
	rm -rf cppcheck
	git clone https://github.com/danmar/cppcheck.git -b $(CPPCHECK_VERSION) --single-branch --depth 1
	cd cppcheck && make -j $(JOBS) MATCHCOMPILER=yes HAVE_RULES=yes CXXFLAGS="-O2 -DNDEBUG -Wall -Wno-sign-compare -Wno-unused-function -Wno-multichar"
	touch cppcheck.done

doxygen.done:
	rm -rf doxygen
	git clone https://github.com/doxygen/doxygen.git -b Release_$(subst .,_,$(DOXYGEN_VERSION)) --single-branch --depth 1
	cd doxygen && mkdir -p build && cd build && cmake -G "Unix Makefiles" .. && make -j $(JOBS)
	touch doxygen.done

shellcheck.done:
	rm -rf shellcheck
	wget https://github.com/koalaman/shellcheck/releases/download/v$(SHELLCHECK_VERSION)/shellcheck-v$(SHELLCHECK_VERSION).linux.x86_64.tar.xz
	tar -xf shellcheck-v$(SHELLCHECK_VERSION).linux.x86_64.tar.xz
	rm -f shellcheck-v$(SHELLCHECK_VERSION).linux.x86_64.tar.xz
	mv shellcheck-v$(SHELLCHECK_VERSION) shellcheck
	touch shellcheck.done

shfmt.done:
	rm -rf shfmt
	mkdir shfmt
	wget -O shfmt/shfmt https://github.com/mvdan/sh/releases/download/v$(SHFMT_VERSION)/shfmt_v$(SHFMT_VERSION)_linux_amd64
	chmod 755 shfmt/shfmt
	touch shfmt.done

sptk.done:
	rm -rf sptk
	export GIT_SSL_NO_VERIFY=1 && git clone https://git.code.sf.net/p/sp-tk/SPTK sptk
	cd sptk/src && aclocal && automake -a -c && autoconf
	cd sptk/src && ./configure --enable-double --prefix=$(CURDIR)/sptk && make -j $(JOBS) && make install
	touch sptk.done

clean:
	rm -rf venv bats cppcheck doxygen shellcheck shfmt sptk *.done

.PHONY: all check venv venv_dev clean
