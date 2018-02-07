all:
		icc counters.c -o count -lpapi -O3 -fp-model precise

clean:
		rm count
		rm output.csv
