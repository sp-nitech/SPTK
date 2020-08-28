.PHONY: all clean

all: virtualenv bats.done doxygen.done sptk.done

virtualenv:
	test -d venv || virtualenv -p python3.6 venv
	. venv/bin/activate; pip install -r requirements.txt
	touch venv/bin/activate

bats.done:
	git clone https://github.com/sstephenson/bats.git
	touch bats.done

doxygen.done:
	git clone https://github.com/doxygen/doxygen.git
	cd doxygen; mkdir build; cd build; cmake -G "Unix Makefiles" ..; make
	touch doxygen.done

sptk.done:
	git clone https://git.code.sf.net/p/sp-tk/SPTK sptk
	cd sptk/src; aclocal; touch ChangeLog; automake -a -c; autoconf; \
	./configure --enable-double --prefix=$(CURDIR)/sptk; make; make install
	touch sptk.done

clean:
	rm -rf venv bats doxygen sptk *.done
