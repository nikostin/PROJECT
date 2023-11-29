CC = gcc
CFLAGS = -Wall -fopenmp
OBJS = main.o partA.o partB.o assignment2.o \
       parse/convertMNIST.o parse/parseMNIST.o lists/List.o vectors/Vector.o \
       search/lsh/lsh.o search/hypercubes/hypercube.o search/distancemetrics/euclidean.o \
       search/hashing/hashing.o search/bruteforce/exhaustive.o \
       clustering/cluster.o clustering/initialization/kmeanspp.o \
       clustering/update/macqueen.o clustering/assignment/lloyd.o \
       clustering/assignment/lsh_reverse_search.o clustering/assignment/hypercube_reverse_search.o \
       clustering/evaluation/silhouette.o search/gnns/gnns.o search/mrng/mrng.o

all: main
	ln -sf main lsh
	ln -sf main cube
	ln -sf main cluster
	ln -sf main graph_search

main: $(OBJS)
	$(CC) $(CFLAGS) -o main $(OBJS) -lm

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

partA.o: partA.c
	$(CC) $(CFLAGS) -c partA.c

partB.o: partB.c
	$(CC) $(CFLAGS) -c partB.c

assignment2.o: assignment2.c
	$(CC) $(CFLAGS) -c assignment2.c -o assignment2.o

parse/convertMNIST.o: parse/convertMNIST.c
	$(CC) $(CFLAGS) -c parse/convertMNIST.c -o parse/convertMNIST.o

parse/parseMNIST.o: parse/parseMNIST.c
	$(CC) $(CFLAGS) -c parse/parseMNIST.c -o parse/parseMNIST.o

lists/List.o: lists/List.c
	$(CC) $(CFLAGS) -c lists/List.c -o lists/List.o

vectors/Vector.o: vectors/Vector.c
	$(CC) $(CFLAGS) -c vectors/Vector.c -o vectors/Vector.o

search/lsh/lsh.o: search/lsh/lsh.c
	$(CC) $(CFLAGS) -c search/lsh/lsh.c -o search/lsh/lsh.o

search/hypercubes/hypercube.o: search/hypercubes/hypercube.c
	$(CC) $(CFLAGS) -c search/hypercubes/hypercube.c -o search/hypercubes/hypercube.o

search/distancemetrics/euclidean.o: search/distancemetrics/euclidean.c
	$(CC) $(CFLAGS) -c search/distancemetrics/euclidean.c -o search/distancemetrics/euclidean.o

search/hashing/hashing.o: search/hashing/hashing.c
	$(CC) $(CFLAGS) -c search/hashing/hashing.c -o search/hashing/hashing.o

search/bruteforce/exhaustive.o: search/bruteforce/exhaustive.c
	$(CC) $(CFLAGS) -c search/bruteforce/exhaustive.c -o search/bruteforce/exhaustive.o

clustering/cluster.o: clustering/cluster.c
	$(CC) $(CFLAGS) -c clustering/cluster.c -o clustering/cluster.o

clustering/initialization/kmeanspp.o: clustering/initialization/kmeanspp.c
	$(CC) $(CFLAGS) -c clustering/initialization/kmeanspp.c -o clustering/initialization/kmeanspp.o

clustering/update/macqueen.o: clustering/update/macqueen.c
	$(CC) $(CFLAGS) -c clustering/update/macqueen.c -o clustering/update/macqueen.o

clustering/assignment/lloyd.o: clustering/assignment/lloyd.c
	$(CC) $(CFLAGS) -c clustering/assignment/lloyd.c -o clustering/assignment/lloyd.o

clustering/assignment/lsh_reverse_search.o: clustering/assignment/lsh_reverse_search.c
	$(CC) $(CFLAGS) -c clustering/assignment/lsh_reverse_search.c -o clustering/assignment/lsh_reverse_search.o

clustering/assignment/hypercube_reverse_search.o: clustering/assignment/hypercube_reverse_search.c
	$(CC) $(CFLAGS) -c clustering/assignment/hypercube_reverse_search.c -o clustering/assignment/hypercube_reverse_search.o

clustering/evaluation/silhouette.o: clustering/evaluation/silhouette.c
	$(CC) $(CFLAGS) -c clustering/evaluation/silhouette.c -o clustering/evaluation/silhouette.o

search/gnns/gnns.o: search/gnns/gnns.c
	$(CC) $(CFLAGS) -c search/gnns/gnns.c -o search/gnns/gnns.o

search/mrng/mrng.o: search/mrng/mrng.c
	$(CC) $(CFLAGS) -c search/mrng/mrng.c -o search/mrng/mrng.o

clean:
	rm -f main lsh cube cluster graph_search $(OBJS)
