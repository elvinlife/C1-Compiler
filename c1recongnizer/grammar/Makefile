item=compilationUnit
mode=gui
testfile=myerror2.c1
clean:
	rm *.java *.class *.tokens
C1: C1Parser.g4 C1Lexer.g4
	java org.antlr.v4.Tool *.g4
	javac *.java
test:
	java org.antlr.v4.runtime.misc.TestRig C1 $(item) -$(mode) ../test/test_cases/$(testfile)

