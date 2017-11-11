## Report for lab1-3

##### 陈泳舟 PB15111667

### 实验分析：

修改并完善项目中的`syntax_tree_builder.cpp`，并将`recognizer.cpp`更改为以`compilationUnit`作为开始符号来解析c1文件，生成AST语法树。

### 实验设计：

总的来说，ANTLR分析树编程接口和.g4文法的关系如下：

**A**.对于产生式`a -> …`，`C1Parser.cpp`中的的接口如下：

* 若产生式右侧含一个符号`token`，可通过`AContext`句柄得到`token`对应的句柄，若有多个`token`符号，则得到`token`对应的多个句柄的`std::vector`。
* 若右侧的符号`token`为非终结符，则该句柄为`tokenContext*`，否则句柄为`TerminalNode*`
* 我们需要通过Vistor模式遍历每一个非终结符的句柄直到非终结符，在`visit`函数中构建AST.

**B**.具体构造AST时，在`visit`中：

* 建立*_syntax变量，根据语义填写`pos`和`line`
* 根据该非终结符的产生式遍历其他非终结符的`context`，从而填写其他变量

**C**.某些难点例子：

* visitBlock

非终结符`block`对应的产生式为：

```g4
block:
    LeftBrace (decl | stmt)* RightBrace
```

我们无法根据`declContext`和`stmtContext`对应的`vector`，知道`decl`和`stmt`在block中具体出现的顺序，因此我们需要查看更上一层的函数`getRuleContexts`的调用，最后发现该函数的本质是遍历`ParseTree::children`，于是，我也便利了`children`变量。

`compileUnit`同理。

* visitConstdef

非终结符`constdef`对应的产生式为：

```g4
constdef:
    Identifier Assign exp
    | Identifier LeftBracket exp? RightBracket Assign LeftBrace exp (Comma exp)* RightBrace
```

当该const变量为数组变量时，我们需要判断第一个`exp`到底是数组的第一个初始值，还是指数组的长度。这里我的方法是比较`exp`的个数和`Comma`加1的大小，若相同，则说明未显示声明数组大小，否则第一个`exp`为数组大小。

```c++
auto exps = ctx->exp();
        int ninit = ctx->Comma().size() + 1;
        //explicitly declare the array length
        if (exps.size() == ninit+1)
        {
            result->array_length.reset(visit(exps[0]).as<expr_syntax*>());
            size_t i = 1;
            for (i=1; i < exps.size(); i++)
            {
                auto expression = visit(exps[i]).as<expr_syntax*>();
                result->initializers.push_back(ptr<expr_syntax>(expression));
            }
        }
        //not explicitly declare the array length
        else
        {
            for(auto init : exps)
            {
                auto expression = visit(init).as<expr_syntax*>();
                result->initializers.push_back(ptr<expr_syntax>(expression));
            }
            auto length = new literal_syntax;
            length->number = ninit;
            result->array_length.reset(static_cast<expr_syntax*>(length));
        }
```

`vardef`同理。

* visitConstdecl与visitDecl

由于我们并没有构建`constdecl`对应的`syntax`，而constdecl的产生式如下：

```
constdecl:
    Const Int constdef (Comma constdef)* SemiColon
```

其中只有一种情况，而且该情况里只有非终结符`constdef`，因此visitConstdecl里我们只需返回`visitVardef()`对应的的`vector`即可，在这里即`std::vector<var_def_stmt_syntax*>`。

`vardecl`同理。



## 重点与难点问题：

深入理解ANTLR的分析原理，结合生成的解析器源码以及所调用的ANTLR运行时的实现代码，调研相关的文献，回答：
- ATN的英文全称是什么，它代表什么？
- SLL的英文全称是什么，它和LL、以及ALL(*)的区别是什么? 它们分别怎么对待文法二义、试探回溯？
- 了解并总结enterRecursionRule、unrollRecursionContexts、adaptivePredict函数的作用、接口和主要处理流程。
- 错误处理机制

### Ans1:

* ** ATN** 的英文全称是 **augmented recursive transition network** [<维基>](https://en.wikipedia.org/wiki/Augmented_transition_network)。

  ATN 是一种在形式语言的*操作定义*（operational definition）中使用的一种*图理论结构*（graph theoretic structure），特别用于解析相对复杂的自然语言，并且在人工智能中具有广泛的应用。理论上，ATN可以分析任何复杂程度句子的结构。ATN是建立在有限状态机来解析句子的想法。

### Ans2:

* **SLL**指**Strong LL(k)**，**SLL**的定义是不加回溯判断的**LL(*)**分析器。
* **LL(*)**分析器是一种处理某些上下文无关文法的自顶向下分析器。在解析一个句子时，LL(k)会提前看k个符号。k的大小决定了LL(*)的分析能力。
* **ALL(\*)**,即**Adaptive LL(k)**是一种$LL(k)$的分析方法，它综合了自上而下分析的简洁以及$GLR$的决策能力。具体而言，$LL(k)$采用的是静态分析方法，它在每个决策点暂停，并等预测机制选择了合适的产生式后继续运行。改进了传统**LL(\*)**的前瞻算法。其在碰到多个可选分支的时候，会为每一个分支运行一个子解析器，每一个子解析器都有自己的DFA，这些子解析器以伪并行的方式探索所有可能的路径，当某一个子解析器完成匹配之后，它走过的路径就会被选定，而其他的子解析器会被杀死，本次决策完成。也就是说，**ALL(\*)**解析器会在运行时反复的扫描输入，这是一个牺牲计算资源换取更强解析能力的算法。在最坏的情况下，这个算法的复杂度为O(n4)，它帮助ANTLR在解决歧义与分支决策的时候更加智能。
* 文法二义：SSL、LL 都无法分析二义文法，而 ALL(\*) 分析器可以根据生成式的顺序来部分解决二义性问题。详情见 [<PDF>](http://www.antlr.org/papers/allstar-techreport.pdf)的4.2节。
* 回溯：SSL 从原理上去除了回溯机制，而通常意义上的 LL 分析器会有回溯机制。ALL 分析器如果采用了阶段分析法，在 SLL 阶段失败后，便会有回溯机制。

### Ans3:

在 `C1Parser.cpp` 中，`enterRecursionRule` 和 `unrollRecursionContexts` 在具有左递归的文法规则 `exp` 中出现：

```C++
C1Parser::ExpContext* C1Parser::exp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  C1Parser::ExpContext *_localctx = _tracker.createInstance<ExpContext>(_ctx, parentState);
  C1Parser::ExpContext *previousContext = _localctx;
  size_t startState = 22;
	//enterRecursionRule
  enterRecursionRule(_localctx, 22, C1Parser::RuleExp, precedence);

  size_t _la = 0;

  auto onExit = finally([=] {
	//unrollRecursionContexts
    unrollRecursionContexts(parentContext);
  });
	... 
  return _localctx;
}
```

 `enterRecursionRule` 用于在生成左递归的 Parser Tree 节点时，将现在的 Context 构造为接下来的 Context 的子节点，结合左递归的 Parser Tree 结构，用于处理左递归文法。

`unrollRecursionContexts` 则用于返回当前环境，退出左递归文法。

`adaptivePredict`大概是构建一个DFA，根据优先级进行状态的预测和转移，但是每一次转移都会记住当前的状态用于回溯。由于缺乏文档个人看的不是很明白。

### Ans4:

