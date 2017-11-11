all: multfirst, plus first
multfirst: MultFirst.g4
	java org.antlr.v4.Tool -Dlanguage=Python3 MultFirst.g4
plusfirst: PlusFirst.g4
	java org.antlr.v4.Tool -Dlanguage=Python3 PlusFirst.g4	
unsupport:
	java org.antlr.v4.Tool -Dlanguage=Python3 UnsupportedLeftRecursive.g4
.PHONY: preinstall clean
preinstall:
	pip3 install typing
	pip3 install antlr4-python3-runtime	
clean:
	find . -name '*First*.py' -delete
	rm *.tokens
