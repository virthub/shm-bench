.PHONY: all clean

all:
	$(MAKE) -C eap
	$(MAKE) -C cap 
	$(MAKE) -C dap
	$(MAKE) -C rap

clean:
	$(MAKE) -C eap clean
	$(MAKE) -C cap clean
	$(MAKE) -C dap clean
	$(MAKE) -C rap clean
