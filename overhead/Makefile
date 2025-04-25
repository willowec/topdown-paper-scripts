#all: pe_overhead td_overhead pe_overhead_disrdpmc

#pe_overhead:	pe_overhead.c
#	gcc -o pe_overhead pe_overhead.c -lpapi

#pe_overhead_disrdpmc:	pe_overhead_disrdpmc.c
#	gcc -o pe_overhead_disrdpmc pe_overhead_disrdpmc.c -lpapi


#td_overhead:	td_overhead.c
#	gcc -o td_overhead td_overhead.c -lpapi

all: overhead_disrdpmc overhead_rdpmc overhead_td

overhead_disrdpmc:	overhead_test.c
	gcc -DTEST_DISRDPMC -o overhead_disrdpmc overhead_test.c -lpapi

overhead_rdpmc:	overhead_test.c
	gcc -DTEST_RDPMC -o overhead_rdpmc overhead_test.c -lpapi

overhead_td:	overhead_test.c
	gcc -DTEST_TD -o overhead_td overhead_test.c -lpapi

clean:
#	rm -rf pe_overhead td_overhead pe_overhead_disrdpmc
	rm -rf overhead_disrdpmc overhead_rdpmc overhead_td