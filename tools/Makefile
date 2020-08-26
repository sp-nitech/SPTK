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
	mkdir -p sptk; cd sptk; \
	wget http://downloads.sourceforge.net/sp-tk/SPTK-3.11.tar.gz; \
	tar -zxvf SPTK-3.11.tar.gz; cd SPTK-3.11; \
	./configure --enable-double --prefix=$(CURDIR)/sptk; make; make install
	touch sptk.done

clean:
	rm -rf venv bats doxygen sptk *.done
