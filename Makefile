PHONY: lint

lint: 
	@$(shell npm bin)/textlint articles/*.re
