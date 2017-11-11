'''Expression recognizer.
Handles expression described in expr.g4.
Grammar file should be compiled by antlr4 with option '-Dlanguage=Python3' before executing this.
Module 'antlr4' is required.
'''
import antlr4
from typing import Mapping
grammar = input("Please enter the grammer(\"MultFirst\" or \"PlusFirst\"): ") or "MultFirst"
Listener = grammar + "Listener"
Parser = grammar + "Parser"
Lexer = grammar + "Lexer"
glisten = __import__(Listener)
gparse = __import__(Parser)
glex = __import__(Lexer)

class Listener(glisten.__dict__[Listener]):
    '''Listener doing calculation based on recognized input.
    '''

    def __init__(self, var_value_source: Mapping[str, int]):
        self.var_value_source = var_value_source

    def exitMult(self, ctx):
        ctx0 = ctx.getChild(0)
        ctx2 = ctx.getChild(2)
        ctx.value = ctx0.value * ctx2.value
        ctx.string = '(' + ctx0.string + '*' + ctx2.string + ')'

    def exitNum(self, ctx):
        ctx.value = int(str(ctx.getChild(0)))
        ctx.string = str(ctx.value)

    def exitId(self, ctx):
        id = str(ctx.getChild(0))
        ctx.string = id
        ctx.value = self.var_value_source[id]

    def exitPlus(self, ctx):
        ctx0 = ctx.getChild(0)
        ctx2 = ctx.getChild(2)
        ctx.value = ctx0.value + ctx2.value
        ctx.string = '(' + ctx0.string + '+' + ctx2.string + ')'

    def exitBrac(self, ctx):
        ctx1 = ctx.getChild(1)
        ctx.value = ctx1.value
        ctx.string = ctx1.string

class LazyInputDict(dict):
    '''A lazy dictionary asking for input when a new item is queried.'''

    def __getitem__(self, key):
        try:
            return dict.__getitem__(self, key)
        except KeyError:
            self[key] = int(
                input('Please enter value for variable \'{}\': '.format(key)))
        return dict.__getitem__(self, key)


stream = glex.__dict__[Lexer](antlr4.InputStream(input('Please enter an expression: ')))
PARSER = gparse.__dict__[Parser](antlr4.CommonTokenStream(stream))
PARSER.addParseListener(Listener(LazyInputDict()))
expr = PARSER.expr()
print("The expression with parentheses is:{}".format(expr.string))
print("The value is:{}".format(expr.value))
