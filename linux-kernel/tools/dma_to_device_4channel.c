/*
 * This file is part of the Xilinx DMA IP Core driver tools for Linux
 *
 * Copyright (c) 2016-present,  Xilinx, Inc.
 * All rights reserved.
 *
 * This source code is licensed under BSD-style license (found in the
 * LICENSE file in the root directory of this source tree)
 */

#define _BSD_SOURCE
#define _XOPEN_SOURCE 500
#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "dma_utils.c"

#include <pthread.h>


static struct option const long_opts[] = {
	{"device", required_argument, NULL, 'd'},
	{"address", required_argument, NULL, 'a'},
	{"aperture", required_argument, NULL, 'k'},
	{"size", required_argument, NULL, 's'},
	{"offset", required_argument, NULL, 'o'},
	{"count", required_argument, NULL, 'c'},
	{"data infile", required_argument, NULL, 'f'},
	{"data outfile", required_argument, NULL, 'w'},
	{"help", no_argument, NULL, 'h'},
	{"verbose", no_argument, NULL, 'v'},
	{0, 0, 0, 0}
};

#define DEVICE_NAME_DEFAULT "/dev/xdma0_h2c_0"
#define DEVICE_NAME_DEFAULT_1 "/dev/xdma0_h2c_1"
#define DEVICE_NAME_DEFAULT_2 "/dev/xdma0_h2c_2"
#define DEVICE_NAME_DEFAULT_3 "/dev/xdma0_h2c_3"
#define SIZE_DEFAULT (32)
#define COUNT_DEFAULT (1)

typedef struct dma_send_parameter{

	char *devname;
	int fpga_fd;
	char *buffer;
	uint64_t size;
	uint64_t base;
}Node, *LNode;

void *dma_send(void *arg);
//void *dma_send1(void *arg);
//void *dma_send2(void *arg);
//void *dma_send3(void *arg);

static void usage(const char *name)
{
	int i = 0;

	fprintf(stdout, "%s\n\n", name);
	fprintf(stdout, "usage: %s [OPTIONS]\n\n", name);
	fprintf(stdout, 
		"Write via SGDMA, optionally read input from a file.\n\n");

	fprintf(stdout, "  -%c (--%s) device (defaults to %s)\n",
		long_opts[i].val, long_opts[i].name, DEVICE_NAME_DEFAULT);
	i++;
	fprintf(stdout, "  -%c (--%s) the start address on the AXI bus\n",
		long_opts[i].val, long_opts[i].name);
	i++;
	fprintf(stdout, "  -%c (--%s) memory address aperture\n",
		long_opts[i].val, long_opts[i].name);
	i++;
	fprintf(stdout,
		"  -%c (--%s) size of a single transfer in bytes, default %d,\n",
		long_opts[i].val, long_opts[i].name, SIZE_DEFAULT);
	i++;
	fprintf(stdout, "  -%c (--%s) page offset of transfer\n",
		long_opts[i].val, long_opts[i].name);
	i++;
	fprintf(stdout, "  -%c (--%s) number of transfers, default %d\n",
		long_opts[i].val, long_opts[i].name, COUNT_DEFAULT);
	i++;
	fprintf(stdout, "  -%c (--%s) filename to read the data from.\n",
		long_opts[i].val, long_opts[i].name);
	i++;
	fprintf(stdout,
		"  -%c (--%s) filename to write the data of the transfers\n",
		long_opts[i].val, long_opts[i].name);
	i++;
	fprintf(stdout, "  -%c (--%s) print usage help and exit\n",
		long_opts[i].val, long_opts[i].name);
	i++;
	fprintf(stdout, "  -%c (--%s) verbose output\n",
		long_opts[i].val, long_opts[i].name);
	i++;

	fprintf(stdout, "\nReturn code:\n");
	fprintf(stdout, "  0: all bytes were dma'ed successfully\n");
	fprintf(stdout, "  < 0: error\n\n");
}

int main(int argc, char *argv[])
{
	int cmd_opt;
	char *device0 = DEVICE_NAME_DEFAULT;
        char *device1 = DEVICE_NAME_DEFAULT_1;
        char *device2 = DEVICE_NAME_DEFAULT_2;
        char *device3 = DEVICE_NAME_DEFAULT_3;
	uint64_t address = 0;
        uint64_t address_1 = 0x10000000;
        uint64_t address_2 = 0x20000000;
        uint64_t address_3 = 0x30000000;
	uint64_t aperture = 0;
	uint64_t size = SIZE_DEFAULT;
	uint64_t size0 = 0;
        uint64_t size1 = 0;
        uint64_t size2 = 0;
        uint64_t size3 = 0;
	uint64_t offset = 0;
	uint64_t count = COUNT_DEFAULT;
	char *infname = NULL;
        char *ofname = NULL;
        
        long total_time = 0;
        float result;
	float avg_time = 0;
        struct timespec ts_start, ts_end;
	ssize_t rc;
        pthread_t pthread0 = 0;
        pthread_t pthread1 = 0; 
        pthread_t pthread2 = 0; 
        pthread_t pthread3 = 0;  
	int infile_fd = -1;
	char *buffer0 = NULL;
        char *buffer1 = NULL;
        char *buffer2 = NULL;
        char *buffer3 = NULL;
	char *allocated = NULL;
        char *allocated_1 = NULL;
        char *allocated_2 = NULL;
        char *allocated_3 = NULL;
	int fpga_fd0 = open("/dev/xdma0_h2c_0", O_RDWR);
        if (fpga_fd0 < 0) {
		fprintf(stderr, "unable to open device /dev/xdma0_h2c_0, %d.\n",
			fpga_fd0);
		perror("open device");
		return -EINVAL;
	}
        int fpga_fd1 = open("/dev/xdma0_h2c_1", O_RDWR);
        if (fpga_fd1 < 0) {
		fprintf(stderr, "unable to open device /dev/xdma0_h2c_1, %d.\n",
			fpga_fd1);
		perror("open device");
		return -EINVAL;
	}
        int fpga_fd2 = open("/dev/xdma0_h2c_2", O_RDWR);
        if (fpga_fd1 < 0) {
		fprintf(stderr, "unable to open device /dev/xdma0_h2c_2, %d.\n",
			fpga_fd2);
		perror("open device");
		return -EINVAL;
	}
        int fpga_fd3 = open("/dev/xdma0_h2c_3", O_RDWR);
        if (fpga_fd1 < 0) {
		fprintf(stderr, "unable to open device /dev/xdma0_h2c_3, %d.\n",
			fpga_fd3);
		perror("open device");
		return -EINVAL;
	}	

        LNode t0;
        LNode t1;
        LNode t2;
        LNode t3;
        t0 = (LNode)malloc(sizeof(Node));
//        t0->devname = (char *)malloc(20);

        t1 = (LNode)malloc(sizeof(Node));
//        t1->devname = (char *)malloc(20);
#if 0
        t2 = (LNode)malloc(sizeof(Node));
        t2->devname = (char *)malloc(20);

        t3 = (LNode)malloc(sizeof(Node));
        t3->devname = (char *)malloc(20);
#endif
	while ((cmd_opt =
		getopt_long(argc, argv, "vhc:f:d:a:k:s:o:w:", long_opts,
			    NULL)) != -1) {
		switch (cmd_opt) {
		case 0:
			/* long option */
			break;
		case 'd':
			/* device node name */
			//fprintf(stdout, "'%s'\n", optarg);
			device0 = strdup(optarg);
			break;
		case 'a':
			/* RAM address on the AXI bus in bytes */
			address = getopt_integer(optarg);
			break;
		case 'k':
			/* memory aperture windows size */
			aperture = getopt_integer(optarg);
			break;
		case 's':
			/* size in bytes */
			size = getopt_integer(optarg);
			break;
		case 'o':
			offset = getopt_integer(optarg) & 4095;
			break;
			/* count */
		case 'c':
			count = getopt_integer(optarg);
			break;
			/* count */
		case 'f':
			infname = strdup(optarg);
			break;
		case 'w':
			ofname = strdup(optarg);
			break;
			/* print usage help and exit */
		case 'v':
			verbose = 1;
			break;
		case 'h':
		default:
			usage(argv[0]);
			exit(0);
			break;
		}
	}
	if (verbose)
		fprintf(stdout, 
		"dev %s, addr 0x%lx, aperture 0x%lx, size 0x%lx, offset 0x%lx, "
	        "count %lu\n",device0, address, aperture, size, offset, count);
	size0 = size/2;
        size1 = size/2;
//        size2 = size/4;
//        size3 = size/4;
	if (infname) {
		infile_fd = open(infname, O_RDONLY);
		if (infile_fd < 0) {
			fprintf(stderr, "unable to open input file %s, %d.\n",
				infname, infile_fd);
			perror("open input file");
			rc = -EINVAL;
			goto out;
		}
	}

	posix_memalign((void **)&allocated, 4096 /*alignment */ , size0 + 4096);
	if (!allocated) {
		fprintf(stderr, "OOM %lu.\n", size0 + 4096);
		rc = -ENOMEM;
		goto out;
	}
	buffer0 = allocated + offset;
	if (verbose)
		fprintf(stdout, "host buffer 0x%lx = %p\n",
			size0 + 4096, buffer0); 

        posix_memalign((void **)&allocated_1, 4096 /*alignment */ , size1 + 4096);
	if (!allocated_1) {
		fprintf(stderr, "OOM %lu.\n", size1 + 4096);
		rc = -ENOMEM;
		goto out;
	}
	buffer1 = allocated_1 + offset;
	if (verbose)
		fprintf(stdout, "host buffer 0x%lx = %p\n",
			size1 + 4096, buffer1); 
#if 0
        posix_memalign((void **)&allocated_2, 4096 /*alignment */ , size2 + 4096);
	if (!allocated_2) {
		fprintf(stderr, "OOM %lu.\n", size2 + 4096);
		rc = -ENOMEM;
		goto out;
	}
	buffer2 = allocated_2 + offset;
	if (verbose)
		fprintf(stdout, "host buffer 0x%lx = %p\n",
			size2 + 4096, buffer2); 

        posix_memalign((void **)&allocated_3, 4096 /*alignment */ , size3 + 4096);
	if (!allocated_3) {
		fprintf(stderr, "OOM %lu.\n", size3 + 4096);
		rc = -ENOMEM;
		goto out;
	}
	buffer3 = allocated_3 + offset;
	if (verbose)
		fprintf(stdout, "host buffer 0x%lx = %p\n",
			size3 + 4096, buffer3);         
#endif
	if (infile_fd >= 0) {
		rc = read_to_buffer(infname, infile_fd, buffer0, size0, 0);
		if (rc < 0 || rc < size0)
			goto out;
	}
     
        if (infile_fd >= 0) {
		rc = read_to_buffer(infname, infile_fd, buffer1, size1, size0);
		if (rc < 0 || rc < size0)
			goto out;
	}
#if 0
        if (infile_fd >= 0) {
		rc = read_to_buffer(infname, infile_fd, buffer2, size2, 0x20000000);
		if (rc < 0 || rc < size0)
			goto out;
	}

        if (infile_fd >= 0) {
		rc = read_to_buffer(infname, infile_fd, buffer3, size3, 0x30000000);
		if (rc < 0 || rc < size0)
			goto out;
	}
#endif
        t0->devname = device0;
	t0->base = address;
	t0->buffer = buffer0;
	t0->fpga_fd = fpga_fd0;
	t0->size = size0;

        t1->devname = device1;
	t1->base = size0;
	t1->buffer = buffer1;
	t1->fpga_fd = fpga_fd1;
	t1->size = size1;
#if 0
        t2->devname = device2;
	t2->base = address_2;
	t2->buffer = buffer2;
	t2->fpga_fd = fpga_fd2;
	t2->size = size2;

        t3->devname = device3;
	t3->base = address_3;
	t3->buffer = buffer3;
	t3->fpga_fd = fpga_fd3;
	t3->size = size3;
#endif
	//p0 = (void *)t0;
	printf("t0->base = %ld\n",t0->base);
	rc = clock_gettime(CLOCK_MONOTONIC, &ts_start);
	
        pthread_create(&pthread0, NULL, dma_send, (void *)t0);
        printf("thread0 id: %ld\n", pthread0);
	
        pthread_create(&pthread1, NULL, dma_send, (void *)t1);
        printf("thread1 id: %ld\n", pthread1);
#if 0	
        pthread_create(&pthread2, NULL, dma_send, (void *)t2);
        printf("thread2 id: %ld\n", pthread2);
	
        pthread_create(&pthread3, NULL, dma_send, (void *)t3);
        printf("thread3 id: %ld\n", pthread3);
#endif   
        pthread_join(pthread0, NULL);
        pthread_join(pthread1, NULL);
#if 0
        pthread_join(pthread2, NULL);
        pthread_join(pthread3, NULL);
        
#endif       
	rc = clock_gettime(CLOCK_MONOTONIC, &ts_end);
	/* subtract the start time from the end time */
	timespec_sub(&ts_end, &ts_start);
	total_time += ts_end.tv_nsec;
	if (verbose)
	fprintf(stdout,
		" CLOCK_MONOTONIC %ld.%09ld sec. write %ld bytes\n",
		ts_end.tv_sec, ts_end.tv_nsec, size); 

	avg_time = (float)total_time/(float)count;
	result = ((float)size)*1000/avg_time;
	if (verbose)
		printf("** Avg time device , total time %ld nsec, avg_time = %f, size = %lu, BW = %f \n",
	        total_time, avg_time, size, result);
	printf(" ** Average BW = %lu, %f\n", size, result);

out:
	close(fpga_fd0);
	if (infile_fd >= 0)
		close(infile_fd);
        free(t0);
        free(t1);
//       free(t2);
//       free(t3);
//        free(t0->devname);
//        free(t1->devname);
//        free(t2->devname);
//        free(t3->devname);
	free(allocated);
        free(allocated_1);
//        free(allocated_2);
//       free(allocated_3);
	return 0; 
}
void *dma_send(void *arg)
{
        ssize_t rc;
	LNode node = (LNode)arg;
	char *devname;
	int fpga_fd;
	char *buffer;
	uint64_t size;
	uint64_t base;
	
	devname = node->devname;
	fpga_fd = node->fpga_fd;
	buffer = node->buffer;
	size = node->size;
	base = node->base;
        printf("base = 0x%lx\n",base);
        printf("fpga_fd= %d\n",fpga_fd);

	rc = write_from_buffer(devname, fpga_fd, buffer, size,base);
	if (rc < 0){
		printf("dma write faild!\n");
	}	
}
#if 0
void *dma_send1(void *arg)
{
        ssize_t rc;
	LNode node = (LNode)arg;
	char *devname;
	int fpga_fd;
	char *buffer;
	uint64_t size;
	uint64_t base;
	
	devname = node->devname;
	fpga_fd = node->fpga_fd;
	buffer = node->buffer;
	size = node->size;
	base = node->base;
        printf("base 1 = 0x%lx\n",base);
        printf("fpga_fd1= %d\n",fpga_fd);

	rc = write_from_buffer(devname, fpga_fd, buffer, size,base);
	if (rc < 0){
		printf("dma write faild!\n");
	}	
}

void *dma_send2(void *arg)
{
        ssize_t rc;
	LNode node = (LNode)arg;
	char *devname;
	int fpga_fd;
	char *buffer;
	uint64_t size;
	uint64_t base;
	
	devname = node->devname;
	fpga_fd = node->fpga_fd;
	buffer = node->buffer;
	size = node->size;
	base = node->base;

	printf("base 2 = 0x%lx\n",base);
        printf("fpga_fd2 = %d\n",fpga_fd);

	rc = write_from_buffer(devname, fpga_fd, buffer, size,base);
	if (rc < 0){
		printf("dma write faild!\n");
	}	
}

void *dma_send3(void *arg)
{
        ssize_t rc;
	LNode node = (LNode)arg;
	char *devname;
	int fpga_fd;
	char *buffer;
	uint64_t size;
	uint64_t base;
	
	devname = node->devname;
	fpga_fd = node->fpga_fd;
	buffer = node->buffer;
	size = node->size;
	base = node->base;
        
	printf("base 3 = 0x%lx\n",base);
        printf("fpga_fd3 = %d\n",fpga_fd);

	rc = write_from_buffer(devname, fpga_fd, buffer, size,base);
	if (rc < 0){
		printf("dma write faild!\n");
	}	
}
#endif 
