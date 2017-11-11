### 遇到的问题：

* 调用`llvm`builder的函数时，由于文档里对每个`Create`函数没有详细的说明，导致开始出现大量类型错误，即对`int32`类型变量调用`float`类型的比较，例如：

  ```c++
  Value* equal1 = builder.CreateFCmpEQ(n, ConstantInt::get(Type::getInt32Ty(context), 1, true), "equal1");
  ```

* 在fib()函数返回值时`CreateRet()`，开始没有使用`SSA`格式，对`Value* ret`多次赋值，导致`llvm-ir`代码出现问题，后来使用`CreatePHI`得以用`pnret`做为函数正确返回值。

* main函数里的循环逻辑开始出现了问题，求和为143，后来更正。
