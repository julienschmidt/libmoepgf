#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "gf.h"
#include "gf2.h"
#include "gf16.h"
#include "gf256.h"

#define timespecclear(tvp)	((tvp)->tv_sec = (tvp)->tv_nsec = 0)
#define timespecisset(tvp)	((tvp)->tv_sec || (tvp)->tv_nsec)
#define timespeccmp(tvp, uvp, cmp)					\
	(((tvp)->tv_sec == (uvp)->tv_sec) ?				\
	    ((tvp)->tv_nsec cmp (uvp)->tv_nsec) :			\
	    ((tvp)->tv_sec cmp (uvp)->tv_sec))
#define timespecadd(vvp, uvp)						\
	({								\
		(vvp)->tv_sec += (uvp)->tv_sec;				\
		(vvp)->tv_nsec += (uvp)->tv_nsec;			\
		if ((vvp)->tv_nsec >= 1000000000) {			\
			(vvp)->tv_sec++;				\
			(vvp)->tv_nsec -= 1000000000;			\
		}							\
	})
#define timespecsub(vvp, uvp)						\
	({								\
		(vvp)->tv_sec -= (uvp)->tv_sec;				\
		(vvp)->tv_nsec -= (uvp)->tv_nsec;			\
		if ((vvp)->tv_nsec < 0) {				\
			(vvp)->tv_sec--;				\
			(vvp)->tv_nsec += 1000000000;			\
		}							\
	})
#define timespecmset(vvp, msec)						\
	({								\
		(vvp)->tv_sec = msec/1000;				\
		(vvp)->tv_nsec = msec-((vvp)->tv_sec*1000);		\
		(vvp)->tv_nsec *= 1000000;				\
	})
#define timespeccpy(vvp, uvp)						\
	({								\
		(vvp)->tv_sec = (uvp)->tv_sec;				\
		(vvp)->tv_nsec = (uvp)->tv_nsec;			\
	})

#define u8_to_float(x)				\
	({					\
	 	(float)(x)/255.0;               \
	})
#define float_to_u8(x)				\
	({					\
	 	(u8)((x)*255.0);                \
	})


static void
init_test_buffers(uint8_t *test1, uint8_t *test2, uint8_t *test3, int size)
{
	int i;
	for (i=0; i<size; i++) {
		test1[i] = rand();
		test2[i] = test1[i];
		test3[i] = rand();
	}
}

static void
encode(uint8_t *dst, uint8_t **buffer, int len, int count, int field)
{
	int i, c;

	for (i=0; i<count; i++) {
		c = rand();
		__galois_fields[field].fmaddrc(dst, buffer[i], c, len);
	}
}

int
main(int argc, char **argv)
{
	int i;
	struct timespec start, end;
	uint64_t BSIZE = 1024*1024*10;
	int tlen = 64+19;

	fprintf(stdout, "%d\n", __galois_fields[1].polynomial);

	if (argc > 1)
		BSIZE = atoi(argv[1]);

	if (BSIZE <= 0 || BSIZE >= 1024UL*1024UL*1024UL*5UL) {
		fprintf(stderr, "invalid test size");
		exit(-1);
	}

	uint8_t *buffer1, *buffer2, *buffer3;
       	uint8_t	*test1, *test2, *test3;

	if (posix_memalign((void *)&test1, 16, tlen))
		exit(-1);
	if (posix_memalign((void *)&test2, 16, tlen))
		exit(-1);
	if (posix_memalign((void *)&test3, 16, tlen))
		exit(-1);

	fprintf(stderr, "GF16 ffmul self check... ");
	init_test_buffers(test1, test2, test3, tlen);
	for (i=15; i>=0; i--) {
		ffmul16_region_c_slow(test1, i, tlen);
		ffmul16_region_c(test2, i, tlen);
	
		if (memcmp(test1, test2, tlen)) {
			fprintf(stderr,"FAIL: results differ, c = %d\n",i);
			exit(-1);
		}
	}
	fprintf(stderr, "PASS\n");
	
	fprintf(stderr, "GF16 ffmadd self check... ");
	init_test_buffers(test1, test2, test3, tlen);
	for (i=15; i>=0; i--) {
		ffmadd16_region_c_slow(test1, test3, i, tlen);
		ffmadd16_region_c(test2, test3, i, tlen);
	
		if (memcmp(test1, test2, tlen)) {
			fprintf(stderr,"FAIL: results differ, c = %d\n",i);
			exit(-1);
		}
	}
	fprintf(stderr, "PASS\n");
	
	fprintf(stderr, "GF256 ffmul self check... ");
	init_test_buffers(test1, test2, test3, tlen);
	for (i=255; i>=0; i--) {
		ffmul256_region_c_slow(test1, i, tlen);
		ffmul256_region_c(test2, i, tlen);
	
		if (memcmp(test1, test2, tlen)) {
			fprintf(stderr,"FAIL: results differ, c = %d\n",i);
			exit(-1);
		}
	}
	fprintf(stderr, "PASS\n");
	
	fprintf(stderr, "GF256 ffmadd self check... ");
	init_test_buffers(test1, test2, test3, tlen);
	for (i=15; i>=0; i--) {
		ffmadd256_region_c_slow(test1, test3, i, tlen);
		ffmadd256_region_c(test2, test3, i, tlen);
	
		if (memcmp(test1, test2, tlen)) {
			fprintf(stderr,"FAIL: results differ, c = %d\n",i);
			exit(-1);
		}
	}
	fprintf(stderr, "PASS\n");

	free(test1);
	free(test2);
	free(test3);

	fprintf(stderr, "\nallocating buffers for benchmark... ");	
	if (posix_memalign((void *)&buffer1, 16, BSIZE))
		exit(-1);
	if (posix_memalign((void *)&buffer2, 16, BSIZE))
		exit(-1);
	if (posix_memalign((void *)&buffer3, 16, BSIZE))
		exit(-1);

	init_test_buffers(buffer1, buffer2, buffer3, BSIZE);

	fprintf(stderr, "done\n");	

	fprintf(stderr, "\nGF16 ffmul benchmark...\n");
	clock_gettime(CLOCK_MONOTONIC, &start);
	ffmul16_region_c_slow(buffer1, 7, BSIZE);
	clock_gettime(CLOCK_MONOTONIC, &end);
	timespecsub(&end, &start);
	fprintf(stderr, "old: %lu sec %lu nsec\n",
		(uint64_t)end.tv_sec, (uint64_t)end.tv_nsec);
	
	clock_gettime(CLOCK_MONOTONIC, &start);
	__galois_fields[GF16].fmulrc(buffer2, 7, BSIZE);
	clock_gettime(CLOCK_MONOTONIC, &end);
	timespecsub(&end, &start);
	fprintf(stderr, "new: %lu sec %lu nsec\n",
		(uint64_t)end.tv_sec, (uint64_t)end.tv_nsec);

	if (memcmp(buffer1, buffer2, tlen)) {
		fprintf(stderr,"FAIL: results differ");
		exit(-1);
	}

	fprintf(stderr, "\nGF16 ffmadd benchmark...\n");
	clock_gettime(CLOCK_MONOTONIC, &start);
	ffmadd16_region_c_slow(buffer1, buffer3, 7, BSIZE);
	clock_gettime(CLOCK_MONOTONIC, &end);
	timespecsub(&end, &start);
	fprintf(stderr, "old: %lu sec %lu nsec\n",
		(uint64_t)end.tv_sec, (uint64_t)end.tv_nsec);
	
	clock_gettime(CLOCK_MONOTONIC, &start);
	__galois_fields[GF16].fmaddrc(buffer2, buffer3, 7, BSIZE);
	clock_gettime(CLOCK_MONOTONIC, &end);
	timespecsub(&end, &start);
	fprintf(stderr, "new: %lu sec %lu nsec\n",
		(uint64_t)end.tv_sec, (uint64_t)end.tv_nsec);

	if (memcmp(buffer1, buffer2, tlen)) {
		fprintf(stderr,"FAIL: results differ");
		exit(-1);
	}
	
	fprintf(stderr, "\nGF256 ffmul benchmark...\n");
	clock_gettime(CLOCK_MONOTONIC, &start);
	ffmul256_region_c_slow(buffer1, 7, BSIZE);
	clock_gettime(CLOCK_MONOTONIC, &end);
	timespecsub(&end, &start);
	fprintf(stderr, "old: %lu sec %lu nsec\n",
		(uint64_t)end.tv_sec, (uint64_t)end.tv_nsec);
	
	clock_gettime(CLOCK_MONOTONIC, &start);
	__galois_fields[GF256].fmulrc(buffer2, 7, BSIZE);
	clock_gettime(CLOCK_MONOTONIC, &end);
	timespecsub(&end, &start);
	fprintf(stderr, "new: %lu sec %lu nsec\n",
		(uint64_t)end.tv_sec, (uint64_t)end.tv_nsec);

	if (memcmp(buffer1, buffer2, tlen)) {
		fprintf(stderr,"FAIL: results differ");
		exit(-1);
	}

	fprintf(stderr, "\nGF256 ffmadd benchmark...\n");
	clock_gettime(CLOCK_MONOTONIC, &start);
	ffmadd256_region_c_slow(buffer1, buffer3, 7, BSIZE);
	clock_gettime(CLOCK_MONOTONIC, &end);
	timespecsub(&end, &start);
	fprintf(stderr, "old: %lu sec %lu nsec\n",
		(uint64_t)end.tv_sec, (uint64_t)end.tv_nsec);
	
	clock_gettime(CLOCK_MONOTONIC, &start);
	__galois_fields[GF256].fmaddrc(buffer2, buffer3, 7, BSIZE);
	clock_gettime(CLOCK_MONOTONIC, &end);
	timespecsub(&end, &start);
	fprintf(stderr, "new: %lu sec %lu nsec\n",
		(uint64_t)end.tv_sec, (uint64_t)end.tv_nsec);

	if (memcmp(buffer1, buffer2, tlen)) {
		fprintf(stderr,"FAIL: results differ");
		exit(-1);
	}
	
	fprintf(stderr, "\nGF2 ffmadd benchmark... (c=1)\n");
	clock_gettime(CLOCK_MONOTONIC, &start);
	__galois_fields[GF2].fmaddrc(buffer2, buffer3, 1, BSIZE);
	clock_gettime(CLOCK_MONOTONIC, &end);
	timespecsub(&end, &start);
	fprintf(stderr, "new: %lu sec %lu nsec\n",
		(uint64_t)end.tv_sec, (uint64_t)end.tv_nsec);

	free(buffer1);
	free(buffer2);
	free(buffer3);


	int len = 2048;
	int count = 16;
	uint8_t **generation;
	uint8_t *frame;
	double mbps;

	generation = malloc(count*sizeof(uint8_t *));
	for (i=0; i<count; i++) {
		if (posix_memalign((void *)&generation[i], 16, len))
			exit(-1);
	}
	if (posix_memalign((void *)&frame, 16, len))
		exit(-1);

	fprintf(stderr, "\nEncoding benchmark, len=%d, count=%d\n", len, count);

	// dry-run to avoid caching panelties for gf2
	encode(frame, generation, len, count, GF2);
	for (i=0; i<3; i++) {
		clock_gettime(CLOCK_MONOTONIC, &start);
		encode(frame, generation, len, count, i);
		clock_gettime(CLOCK_MONOTONIC, &end);
		timespecsub(&end, &start);
		mbps = 1.0/((double)end.tv_sec + (double)end.tv_nsec*1e-9);
		mbps *= len;
		mbps /= 1024*1024;

		fprintf(stderr, "%lu sec %lu nsec (%.2f MiB/s)\n",
			(uint64_t)end.tv_sec, (uint64_t)end.tv_nsec, mbps);
	}

	return 0;
}