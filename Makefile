.PHONY: all common algo gm sim clean
all: common algo gm sim
	@echo "Build complete."

common:
	@echo "Common headers are header-only; nothing to build."

algo:
	$(MAKE) -C Algorithm

gm:
	$(MAKE) -C GameManager

sim:
	$(MAKE) -C Simulator

clean:
	$(MAKE) -C Algorithm clean
	$(MAKE) -C GameManager clean
	$(MAKE) -C Simulator clean
	@echo "Clean complete."
