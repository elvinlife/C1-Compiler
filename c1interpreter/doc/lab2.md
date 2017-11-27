## Report for lab2

**陈泳舟 PB15111667**

### 实验分析：

使用`visitor pattern`，由之前实验构建的语法树生成`llvm ir`中间代码。实验要求填补`assembly_builder.cpp` 中的所有空白 `visit` 方法，并补全测试样例。

### 实验设计：

总的来说，在预热实验后，生成`llvm ir`中间代码还是较为轻松的。

下面列出一些重难点

* 左值和右值的处理：

  变量的使用分为两种情况：作为左值和作为右值使用。这将由 `lval_as_rval` 这一标志区分。

  - **作为左值使用**

    即作为被赋值的对象。这时，你应当取出其地址，存入 `value_result` 中。你需要检查变量的类型，对数组引用进行正确的索引（通过 `getelementptr` 指令]）。

  - **作为右值使用**

    当作为右值使用时，除了上述步骤外，你还需要将值取出（通过 `load`指令），然后将取出的值所关联的 `Value` 指针存入 `value_result` 中。

  本实验需要用右值的地方有：

  * 局部变量的数组初始化时的初始化列表
  * 访问`assign_stmt_syntax`时，赋值符号的右侧
  * 访问数组元素时，数组的下标
  * 访问`cond_syntax`时，二元比较符的两侧

* 变量定义的处理

  * 局部变量：

    调用IRBuilder::CreateAlloca 创建 `alloca` 指令，在栈上分配空间。局部变量无论是否可变，其初始化表达式均**不要求**是常量表达式。

  * 全局变量

    使用类似于 `new GlobalVariable` 的方式在当前 `module` 中创建一个全局变量。为了简单起见，无论变量为常量或可变量，其初始化表达式均**要求**是常量表达式。

    数组声明时，检查数组长度是否不小于初始化列表长度；若小于，则应通过err.error报错。

* 控制流结构的处理

  生成控制流时注意尽可能生成少的BasicBlock，同时注意不同BasicBlock的跳转，防止出现某个BB后无Return或Branch的情况，这会出现段错误。

  * while 语句

  ```c++
  void assembly_builder::visit(while_stmt_syntax &node)
  {
    BasicBlock* cond_bb = BasicBlock::Create(context, "L", current_function);
    BasicBlock* body_bb = BasicBlock::Create(context, "L", current_function);
    BasicBlock* end_bb = BasicBlock::Create(context, "L", current_function);
    builder.CreateBr(cond_bb);
    builder.SetInsertPoint(cond_bb);
    node.pred->accept(*this);
    builder.CreateCondBr(value_result, body_bb, end_bb);
    builder.SetInsertPoint(body_bb);
    node.body->accept(*this);
    builder.CreateBr(cond_bb);
    builder.SetInsertPoint(end_bb);
  }
  ```
  * if-else 语句

  ```c++
  void assembly_builder::visit(if_stmt_syntax &node)
  {
    BasicBlock* then_bb = BasicBlock::Create(context, "L", current_function);
    BasicBlock* else_bb = BasicBlock::Create(context, "L", current_function);
    BasicBlock* end_bb = BasicBlock::Create(context, "L", current_function);
    node.pred->accept(*this);
    //no else body
    if(!node.else_body)
    {
      builder.CreateCondBr(value_result, then_bb, end_bb);
      builder.SetInsertPoint(then_bb);
      node.then_body->accept(*this);
    }
    //has else body
    else
    {
      builder.CreateCondBr(value_result, then_bb, else_bb);
      builder.SetInsertPoint(then_bb);
      node.then_body->accept(*this);
      builder.CreateBr(end_bb);
      builder.SetInsertPoint(else_bb);
      node.else_body->accept(*this);
    }
    builder.CreateBr(end_bb);
    builder.SetInsertPoint(end_bb);
  }
  ```

* error

  在处理error时，如果当前节点出错，则报错并忽略这一支子树，返回一个占位标记值，继续处理。我们使用 `assembly_builder::error_flag` 作为一个 `builder` 对象的错误标记，在开始生成时重置为假，结束后检查是否为真；若为真，则清空截至目前已生成的IR代码

  具体逻辑：

  当该节点返回一个`value_result`时，出错则将其置为`nullptr`，在上层节点访问`value_result`，若为`nullptr`则自动返回，从而忽略该子树。

  若该节点返回`const_value`，我们不需要进行额外处理，因为这时编译仍然可以正常进行。

### 遇到的问题：

* 在补全`while_stmt_syntax`和`if_stmt_syntax`后生成相应的代码，运行时报段错误

  原因：忘记了每一个BasicBlock之后必须跟一个Branch或者Return指令，补全即可。

* 访问数组元素如果下标如果是变量会报错，而常量表达式则不会

  原因：取下标时忘记设置取变量右值
