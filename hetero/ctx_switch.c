#include <papi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_EVENTS	8

// fibonacci function to serve as a benchable code section
void __attribute__((optimize("O0"))) fib(int n)
{
	long i, a = 0;
	int b = 1;
	for (i = 0; i < n; i++)
	{
		b = b + a;
		a = b - a;
	}
}

int main()
{
	int retval, i;
	const PAPI_component_info_t *cmpinfo = NULL;
	int numcmp, cid, topdown_cid = -1;
	long long values[NUM_EVENTS];
	int EventSet = PAPI_NULL;

	//usec = PAPI_get_real_usec();

	retval=PAPI_library_init(PAPI_VER_CURRENT);
	if (retval!=PAPI_VER_CURRENT) {
		fprintf(stderr,"Error initializing PAPI! %s\n",
		PAPI_strerror(retval));
		return 1;
	}

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

	/* create EventSet */
	retval = PAPI_create_eventset(&EventSet);
	if (retval != PAPI_OK)
	{
		printf("failed to create eventset %d\n", retval);
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

	//printf("init took %lldms\n", (PAPI_get_real_usec()-usec) / 1000);

	//usec = PAPI_get_real_usec();
	/* stat some dummy code */
	for (i=0; i<100; i++) {
		PAPI_start(EventSet);
		fib(500000);
		PAPI_stop(EventSet, values);
	}
	//printf("work loop took %lldms\n", (PAPI_get_real_usec()-usec) / 1000);

	return 0;
}
