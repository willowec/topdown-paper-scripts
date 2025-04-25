// combined overhead test

#include <papi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/perf_event.h>
#include <err.h>

#define N_TESTS	 100000000 

//#define TEST_DISRDPMC
//#define TEST_RDPMC
//#define TEST_TD

#define PROFILE		1 // count an HPC
#define PROF_EVENT	"FRONTEND_RETIRED:L2_MISS"
//#define PROF_EVENT	"ASSISTS:PAGE_FAULT" // no correlation
//#define PROF_EVENT	"DTLB_STORE_MISSES:WALK_COMPLETED" // no correlation at all
//#define PROF_EVENT	"FRONTEND_RETIRED:STLB_MISS" // no corelation at all

//#define PROF_EVENT	"L2_REQUEST:ALL_DEMAND_MISS" // a bit of correlation with some spikes
//#define PROF_EVENT	"L1D:REPLACEMENT" // definitely some coorelation
//#define PROF_EVENT	"L1D:HWPF_MISS" // definitely some coorelation

//#define PROFILE_CTX	1	// count context switches


#ifdef TEST_DISRDPMC
#define NUM_EVENTS	8
#endif
#ifdef TEST_RDPMC
#define NUM_EVENTS	2
#endif
#ifdef TEST_TD
#define NUM_EVENTS	12
#endif

long long cycle_counts[N_TESTS] = {0};
long long profile_counts[N_TESTS] = {0};
long long profile_counts2[N_TESTS] = {0};

/* quicksort sort function */
int comp(const void *a, const void *b)
{
	return *(long long*)a - *(long long*)b;
}

static long
perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
				int cpu, int group_fd, unsigned long flags)
{
	int ret;

	ret = syscall(SYS_perf_event_open, hw_event, pid, cpu,
				  group_fd, flags);
	return ret;
}

int main()
{
	int retval, i, j, k;
	const PAPI_component_info_t *cmpinfo = NULL;
	int numcmp, cid, topdown_cid = -1;
	long long values[NUM_EVENTS];
	long long tmp, cyc_sum, cyc_Q0, cyc_Q1, cyc_Q2, cyc_Q3, cyc_Q4;
	int EventSet = PAPI_NULL;

	retval=PAPI_library_init(PAPI_VER_CURRENT);
	if (retval!=PAPI_VER_CURRENT) {
		fprintf(stderr,"Error initializing PAPI! %s\n",
		PAPI_strerror(retval));
		return 1;
	}

#ifdef TEST_TD
	/* find the topdown component */
	numcmp = PAPI_num_components();
	for (cid = 0; cid < numcmp; cid++)
	{
		if ((cmpinfo = PAPI_get_component_info(cid)) == NULL)
		{
			printf("Failed to find topdown component\n");
			return 1;
		}
		if (strstr(cmpinfo->name, "topdown"))
		{
			topdown_cid = cid;

			/* check that the component is enabled */
			if (cmpinfo->disabled)
			{
				printf("Topdown component is disabled: %s\n", cmpinfo->disabled_reason);
				return 1;
			}
		}
	}

	/* ensure topdown component was found */
	if (topdown_cid < 0)
	{
		printf("Topdown component not found\n");
		return 1;
	}
#endif
	/* create EventSet */
	retval = PAPI_create_eventset(&EventSet);
	if (retval != PAPI_OK)
	{
		printf("failed to create eventset %d\n", retval);
		return 1;
	}

#ifdef TEST_DISRDPMC

	/* add slots and enough events to get l1 and l2 */
	
	retval = PAPI_add_named_event(EventSet, "TOPDOWN:SLOTS");
	if (retval != PAPI_OK) {
		return 1;
	}

	retval = PAPI_add_named_event(EventSet, "TOPDOWN:RETIRING_SLOTS");
	if (retval != PAPI_OK) {
		return 1;
	}
	retval = PAPI_add_named_event(EventSet, "TOPDOWN:BAD_SPEC_SLOTS");
	if (retval != PAPI_OK) {
		return 1;
	}
	retval = PAPI_add_named_event(EventSet, "TOPDOWN:BACKEND_BOUND_SLOTS");
	if (retval != PAPI_OK) {
		return 1;
	}

	retval = PAPI_add_named_event(EventSet, "UOPS_RETIRED:HEAVY");
	if (retval != PAPI_OK) {
		return 1;
	}
	retval = PAPI_add_named_event(EventSet, "TOPDOWN:BR_MISPREDICT_SLOTS");
	if (retval != PAPI_OK) {
		return 1;
	}
	retval = PAPI_add_named_event(EventSet, "IDQ_UOPS_NOT_DELIVERED:"
		"CYCLES_0_UOPS_DELIV_CORE");
	if (retval != PAPI_OK) {
		return 1;
	}
	retval = PAPI_add_named_event(EventSet, "TOPDOWN:MEMORY_BOUND_SLOTS");
	if (retval != PAPI_OK) {
		return 1;
	}


	printf("make sure you configure with perf_event rdpmc DISABLED\n");

#endif

#ifdef TEST_RDPMC
	/* add slots and one event that, with rdpmc enabled, will get all of */
	/* of the metrics */

	retval = PAPI_add_named_event(EventSet, "TOPDOWN:SLOTS");
	if (retval != PAPI_OK) {
		printf("couldnt add slots\n");
		return 1;
	}

	retval = PAPI_add_named_event(EventSet, "TOPDOWN:RETIRING_SLOTS");
	if (retval != PAPI_OK) {
		printf("couldnt add retiring\n");
		return 1;
	}

	printf("make sure you configure with perf_event rdpmc ENABLED\n");
#endif

#ifdef TEST_TD
	/* add level 1 topdown metrics */
    retval = PAPI_add_named_event(EventSet, "TOPDOWN_RETIRING_PERC");
	if (retval != PAPI_OK)
	{
		return 1;
	}
	retval = PAPI_add_named_event(EventSet, "TOPDOWN_BAD_SPEC_PERC");
	if (retval != PAPI_OK)
	{
		return 1;
	}
	retval = PAPI_add_named_event(EventSet, "TOPDOWN_FE_BOUND_PERC");
	if (retval != PAPI_OK)
	{
		return 1;
	}
	retval = PAPI_add_named_event(EventSet, "TOPDOWN_BE_BOUND_PERC");
	if (retval != PAPI_OK)
	{
		return 1;
	}

	/* add the level 2 topdown metrics */
	retval = PAPI_add_named_event(EventSet, "TOPDOWN_HEAVY_OPS_PERC");
	if (retval != PAPI_OK)
	{
		return 1;
	}
	retval = PAPI_add_named_event(EventSet, "TOPDOWN_LIGHT_OPS_PERC");
	if (retval != PAPI_OK)
	{
		return 1;
	}
	retval = PAPI_add_named_event(EventSet, "TOPDOWN_BR_MISPREDICT_PERC");
	if (retval != PAPI_OK)
	{
		return 1;
	}
	retval = PAPI_add_named_event(EventSet, "TOPDOWN_MACHINE_CLEARS_PERC");
	if (retval != PAPI_OK)
	{
		return 1;
	}
	retval = PAPI_add_named_event(EventSet, "TOPDOWN_FETCH_LAT_PERC");
	if (retval != PAPI_OK)
	{
		return 1;
	}
	retval = PAPI_add_named_event(EventSet, "TOPDOWN_FETCH_BAND_PERC");
	if (retval != PAPI_OK)
	{
		return 1;
	}
	retval = PAPI_add_named_event(EventSet, "TOPDOWN_MEM_BOUND_PERC");
	if (retval != PAPI_OK)
	{
		return 1;
	}
	retval = PAPI_add_named_event(EventSet, "TOPDOWN_CORE_BOUND_PERC");
	if (retval != PAPI_OK)
	{
		return 1;
	}
#endif

#ifdef PROFILE
	/* handle measuring additional HPCs */
	int EventSet2 = PAPI_NULL;

	/* create EventSet */
	retval = PAPI_create_eventset(&EventSet2);
	if (retval != PAPI_OK)
	{
		printf("failed to create eventset 2: %d\n", retval);
		return 1;
	}
	retval = PAPI_add_named_event(EventSet2, PROF_EVENT);
	if (retval != PAPI_OK)
	{
		printf("failed to add " PROF_EVENT " to eventset 2: %d\n", retval);
		return 1;
	}

#endif

#ifdef PROFILE_CTX
	int                     fd;
	long long               count;
	struct perf_event_attr  pe;

	memset(&pe, 0, sizeof(pe));
	pe.type = PERF_TYPE_HW_CACHE;
	pe.size = sizeof(pe);
	pe.config = PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
	pe.disabled = 1;
	pe.exclude_kernel = 1; // need to include kernel for ctx switch
	pe.exclude_hv = 1;

	fd = perf_event_open(&pe, 0, -1, -1, 0);
	if (fd == -1)
		err(EXIT_FAILURE, "Error opening leader %llx\n", pe.config);

#endif

	/* measure overhead */
	for(i=0; i<N_TESTS; i++) {
		if (i%(N_TESTS/10) == 0) printf("%d/%d...\n", i, N_TESTS);

#ifdef PROFILE
		/* also measure the PROF_EVENT */
		PAPI_start(EventSet2);
#endif
#ifdef PROFILE_CTX	
		if (ioctl(fd, PERF_EVENT_IOC_ENABLE, 0) == -1)
			err(EXIT_FAILURE, "PERF_EVENT_IOC_ENABLE");
		if (ioctl(fd, PERF_EVENT_IOC_RESET, 0) == -1)
			err(EXIT_FAILURE, "PERF_EVENT_IOC_RESET");
#endif

		tmp = PAPI_get_real_cyc();
		PAPI_start(EventSet);
		// work function would go here
		PAPI_stop(EventSet, values);
		cycle_counts[i] = PAPI_get_real_cyc() - tmp;
		
#ifdef PROFILE
		PAPI_stop(EventSet2, profile_counts+i);
#endif
#ifdef PROFILE_CTX
		if (ioctl(fd, PERF_EVENT_IOC_DISABLE, 0) == -1)
			err(EXIT_FAILURE, "PERF_EVENT_IOC_DISABLE");
		if (read(fd, profile_counts+i, sizeof(profile_counts[i])) != sizeof(profile_counts[i]))
			err(EXIT_FAILURE, "read");

#endif
	}
#ifdef PROFILE_CTX
	if (close(fd) == -1)
		err(EXIT_FAILURE, "close");
#endif

/* write temporally ordered results to disk */
	FILE *f;
	f = fopen("./over_out.json", "w");
	fprintf(f, "{\n\t\"iterations\": [\n\t\t");
	for(i=0; i<N_TESTS-1; i++) {
		fprintf(f, "%lld,", cycle_counts[i]);
	}
	fprintf(f, "%lld],\n", cycle_counts[N_TESTS]);

#if defined(PROFILE) || defined(PROFILE_CTX)
	/* write out profile event data as well */
	fprintf(f, "\n\t\"prof_iterations\": [\n\t\t");
	for(i=0; i<N_TESTS-1; i++) {
		fprintf(f, "%lld,", profile_counts[i]);
	}
	fprintf(f, "%lld],\n", profile_counts[N_TESTS]);
#endif
	
	/* sort to get sorted results for quartile calcs */
	qsort(cycle_counts, N_TESTS, sizeof(unsigned long long), comp);

	/* report statistics */
	cyc_sum = 0;
	for(i=0; i<N_TESTS; i++) {
		cyc_sum += cycle_counts[i];
	}

	cyc_Q0 = cycle_counts[0];
	cyc_Q1 = cycle_counts[(N_TESTS/2)-(N_TESTS/4)];
	cyc_Q2 = cycle_counts[N_TESTS/2];
	cyc_Q3 = cycle_counts[(N_TESTS/2)+(N_TESTS/4)];
	cyc_Q4 = cycle_counts[N_TESTS-1];

	printf("Min\tQ1\tMedian\tQ3\tMax\n");
	printf("%llu\t%llu\t%llu\t%llu\t%llu\n", cyc_Q0, cyc_Q1, cyc_Q2, cyc_Q3, cyc_Q4);

	/* and also report to file */
	fprintf(f, "\t\"Q0\": %d,\n", cyc_Q0);
	fprintf(f, "\t\"Q1\": %d,\n", cyc_Q1);
	fprintf(f, "\t\"Q2\": %d,\n", cyc_Q2);
	fprintf(f, "\t\"Q3\": %d,\n", cyc_Q3);
	fprintf(f, "\t\"Q4\": %d\n", cyc_Q4);
	fprintf(f, "}");
	fclose(f);
}
