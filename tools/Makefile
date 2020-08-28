.PHONY: all clean

all: virtualenv bats.done cpplint.done doxygen.done sptk.done

virtualenv:
	test -d venv || virtualenv -p python3.6 venv
	. venv/bin/activate; pip install -r requirements.txt
	touch venv/bin/activate

bats.done:
	rm -rf bats
	git clone https://github.com/sstephenson/bats.git
	touch bats.done

cpplint.done:
	rm -rf cpplint
	git clone https://github.com/cpplint/cpplint.git
	touch cpplint.done

doxygen.done:
	rm -rf doxygen
	git clone https://github.com/doxygen/doxygen.git
	cd doxygen; mkdir -p build; cd build; cmake -G "Unix Makefiles" ..; make
	touch doxygen.done

sptk.done:
	rm -rf sptk
	git clone https://git.code.sf.net/p/sp-tk/SPTK sptk
	cd sptk/src; aclocal; touch ChangeLog; automake -a -c; autoconf; \
	./configure --enable-double --prefix=$(CURDIR)/sptk; make; make install
	touch sptk.done

clean:
	rm -rf venv bats cpplint doxygen sptk *.done
