##Answer for HW1

##### 陈泳舟 PB15111667

### 源代码:

```c
int sum(int a, int b)
{
	int sum = a + b;
	return sum;
}

int main()
{
	int a,b;
	a = 1;
	b = 2;
	int c = sum(a, b);
	return -1;
}
```

## 汇编码：

* 编译命令:

  ```sh
  clang -S -fno-asynchronous-unwind-tables -m32 sum.c -o sum_32.s
  ```

* 编译选项的解释：

  * -S: 执行预处理，语法分析，类型检查步骤，并启动llvm优化并编译出汇编码
  * -fno-asynchronous-unwind-tables: 取消异步操作（例如debug，垃圾回收等）中需要的栈地址记录。（为了使汇编码更简洁易懂）
  * -m32:按32位的架构编译源码

* 汇编码：

  ```asm
  	.section	__TEXT,__text,regular,pure_instructions
  	.macosx_version_min 10, 12
  	.globl	_sum
  	.p2align	4, 0x90
  _sum:                                   	## @sum函数
  ## BB#0:
  	pushl	%ebp				#建立sum()函数堆栈框架
  	movl	%esp, %ebp
  	subl	$12, %esp
  	movl	12(%ebp), %eax			#获取形参a
  	movl	8(%ebp), %ecx			#获取形参b
  	movl	%ecx, -4(%ebp)
  	movl	%eax, -8(%ebp)
  	movl	-4(%ebp), %eax
  	addl	-8(%ebp), %eax			#执行sum=a+b的加运算
  	movl	%eax, -12(%ebp)
  	movl	-12(%ebp), %eax			#sum函数返回值存储在eax里
  	addl	$12, %esp
  	popl	%ebp				#拆除堆栈框架
  	retl

  	.globl	_main
  	.p2align	4, 0x90
  _main:					# @main函数
  ## BB#0:
  	pushl	%ebp				#建立main函数堆栈框架
  	movl	%esp, %ebp			#将ebp入栈，ebp存储当前esp值
  	subl	$24, %esp			#为栈预留24个字节
  	movl	$0, -4(%ebp)
  	movl	$1, -8(%ebp)			#变量a
  	movl	$2, -12(%ebp)			#变量b
  	movl	-8(%ebp), %eax
  	movl	-12(%ebp), %ecx
  	movl	%eax, (%esp)			#拷贝变量a为sum实参
  	movl	%ecx, 4(%esp)			#拷贝变量b为sum实参
  	calll	_sum				#调用sum()
  	movl	$4294967295, %ecx      		## imm = 0xFFFFFFFF
  	movl	%eax, -16(%ebp)			#变量c
  	movl	%ecx, %eax			#main函数返回值-1存在eax里
  	addl	$24, %esp
  	popl	%ebp				#拆除堆栈框架
  	retl


  .subsections_via_symbols
  ```

  ​
