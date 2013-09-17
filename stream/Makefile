#
# Construction des programmes:
#
all: clean
	gcc -O stream.c -o stream
	gcc -O -fopenmp stream.c -o stream_omp
#
# Nettoyage:
#
clean:
	rm -f *.o *~ core stream stream_omp
