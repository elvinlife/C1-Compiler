### 完成C语言到LLVM IR的人工翻译：

* 定义函数fib()：

  在实现`if... then... elseif ...else`模块时，为了不破坏原来的C1语言的逻辑，同时使llvm ir尽量高效，我依然设置了5个基本块begin, then, elif, else, ret。

  ```c++
   define i32 @fib(i32 %n) {
   begin:
     %equal0 = icmp eq i32 %n, 0
     br i1 %equal0, label %then, label %elif
   then:                                             ; preds = %begin
     br label %ret
   elif:                                             ; preds = %begin
     %equal1 = icmp eq i32 %n, 1
     br i1 %equal1, label %ret, label %else
   else:                                             ; preds = %elif
     ...
   ret:                                              ; preds = %else, %elif, %then
     %ret1 = phi i32 [ %n, %then ], [ %n, %elif ], [ %sum, %else ]
     ret i32 %ret1
   }
  ```

  最后通过$\phi$函数决定返回值应该取哪一个Value。

* 定义函数main():

  main()函数里主要的工作在实现`for`循环

  首先用`alloca`指令在栈上保存%x局部变量，在`loop`时，我们的计数器`i`从`entry`模块进入时初始化为0，从`loop`模块进入时为i+1，最后比较i与10，决定是否跳出循环。

  ```c++
   define i32 @main() {
   entry:
     %x = alloca i32
     store i32 0, i32* %x
     br label %loop
   loop:                                             ; preds = %loop, %entry
     %i = phi i32 [ 0, %entry ], [ %addi, %loop ]
     ...
     br i1 %if_continue, label %loop, label %end

   end:                                              ; preds = %loop
     %ret = load i32, i32* %x
     ret i32 %ret
   }
  ```

  运行`lli fib.ll`返回值确实为88，实验成功。

### 编写fib_gen：

​	fib_gen.cpp的实现基本上和fib.ll中的llvm ir语言实现的逻辑相同。所要做的就是在文档中找到对应的`c++`语言对应的函数等等，这里就不赘述。

### 遇到的问题：

* 调用`llvm`builder的函数时，由于文档里对每个`Create`函数没有详细的说明，导致开始出现大量类型错误，即对`int32`类型变量调用`float`类型的比较，例如：

  ```c++
  Value* equal1 = builder.CreateFCmpEQ(n, ConstantInt::get(Type::getInt32Ty(context), 1, true), "equal1");
  ```

* 在fib()函数返回值时`CreateRet()`，开始没有使用`SSA`格式，对`Value* ret`多次赋值，导致`llvm-ir`代码出现问题，后来使用`CreatePHI`得以用`pnret`做为函数正确返回值。

* main函数里的循环逻辑开始出现了问题，求和为143，后来更正。
