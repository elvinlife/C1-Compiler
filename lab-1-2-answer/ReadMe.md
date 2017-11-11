###依赖环境：

本程序需要安装Python3 和依赖模块`typing`和`antlr4`，在shell输入下列指令即可：

```shell
make preinstall
```

### 编译环境：

在Linux或者Maxos系统下，你需要将`org.antlr.v4.Tool`添加到`CLASSPATH`环境变量中,输入shell命令

```shell
make all
```

即可生成本实验要求的两种语法(MultFirst与PlusFirst)对应的python模块。

### 使用方式：

在shell运行程序`expr_calculator.py`即可。

程序运行时，首先需要输入你需要测试的语法：`MultFirst` 或`PlusFirst`(缺省是`MultFirst`)

然后输入需要分析的表达式:(例如：`(a+b)+c*2`)

<img src="ex1.png">

<img src="ex2.png">
###分析内容：
* 由上述例子可知，在分析上述两种文法时，antlr4支持直接左递归。

* 在`expr`的产生式出现左递归的情况下，运算符的优先级从上到下依次降低。将`expr Multiply expr`放在`expr  Plus expr`之前，则生成的AST树优先结合`*`运算符。

* 考虑`UnsupportedLeftRecursive.g4`,有产生式：

  ```antler
  expr: s 'a';
  s: expr 'b' | Identifier;
  ```

  该文法有含间接左递归的产生式，且可以化简为直接左递归，但是antlr4并不支持这种该文法，报错：

  `error(119): UnsupportedLeftRecursive.g4::: The following sets of rules are mutually left-recursive [expr, s]`

### 结论：

antlr4可以支持直接左递归文法，在出现左递归时，运算符的优先级从上到下依次降低。但是antlr4并不支持间接左递归。