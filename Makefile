all:
	$(MAKE) -C mem_alloc
	$(MAKE) -C pebs_tests
	$(MAKE) -C cache_tests

clean:
	$(MAKE) -C mem_alloc clean
	$(MAKE) -C pebs_tests clean
	$(MAKE) -C cache_tests clean
