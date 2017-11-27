### Question:

数组访问越界导致的bug一直是C语言中最常见的错误之一，它常常造成各种未定义行为，严重时会引起程序崩溃。代码c1.c是一段访问了下标越界的错误程序，undefined.s是在macOS Sierra 10.12.6操作系统下，由clang-900.0.38编译出的汇编代码，flags为`-m32 -fno-stack-protector`，其中`-fno-stack-protector`是为了使汇编代码更加易懂，`-m32`是为了实验方便。ubsan_error.s是再添加了`-fsanitize=bounds`的汇编代码。

(1)试解释为什么由于访问了a[-2]，最后引起segmentation fault。(考察编译时存储空间的管理)

(2)llvm引入了UndefinedBehaviorSanitizer(UBSan)，一个快速的探测器来防止未定义行为的发生。UBSan通过编译时更改代码，从而捕获到各种执行中的未定义行为。通过添加flag`-fsanitize=pointer-overflow`，程序在访问数组的下标为越界时会报错。试通过生成汇编代码，分析UBSan如何判断下标越界的错误。（考察编译时控制流）

* c1.c

```c
void main()
 {
     int b = 0;
     int *pb = &b;
     int a[] = {1,2,3};
     a[3] = 4;
     int c = *pb;
 }
```

* undefined.s

```asm
_main:
	## BB#0:
    	pushl   %ebp
    	movl    %esp, %ebp
    	subl    $24, %esp
    	calll   L0$pb
	L0$pb:
    	popl    %eax
    	leal    -4(%ebp), %ecx
    	movl    $0, -4(%ebp)
    	movl    %ecx, -8(%ebp)
    	movl    l_main.a-L0$pb(%eax), %ecx
    	movl    %ecx, -20(%ebp)
    	movl    (l_main.a-L0$pb)+4(%eax), %ecx
    	movl    %ecx, -16(%ebp)
    	movl    (l_main.a-L0$pb)+8(%eax), %eax
    	movl    %eax, -12(%ebp)
    	movl    $4, -8(%ebp)
    	movl    -8(%ebp), %eax
    	movl    (%eax), %eax
    	movl    %eax, -24(%ebp)
    	addl    $24, %esp
    	popl    %ebp
    	retl
    	.section    __TEXT,__const
    	.p2align    2               ## @main.a
	l_main.a:
    	.long   1                       ## 0x1
    	.long   2                       ## 0x2
    	.long   3                       ## 0x3
```

* ubsan_error.s

```assembly
    calll   L0$pb
L0$pb:
    popl    %eax
    xorl    %ecx, %ecx
    movb    %cl, %dl
    leal    -4(%ebp), %ecx
    movl    $0, -4(%ebp)
    movl    %ecx, -8(%ebp)
    movl    l_main.a-L0$pb(%eax), %ecx
    movl    %ecx, -20(%ebp)
    movl    (l_main.a-L0$pb)+4(%eax), %ecx
    movl    %ecx, -16(%ebp)
    movl    (l_main.a-L0$pb)+8(%eax), %ecx
    movl    %ecx, -12(%ebp)
    testb   $1, %dl
    movl    %eax, -28(%ebp)         ## 4-byte Spill
    jne LBB0_2
## BB#1:
    movl    -28(%ebp), %eax         ## 4-byte Reload
    leal    l___unnamed_1-L0$pb(%eax), %ecx
    movl    $3, %edx
    movl    %ecx, (%esp)
    movl    $3, 4(%esp)
    movl    %edx, -32(%ebp)         ## 4-byte Spill
    calll   ___ubsan_handle_out_of_bounds
LBB0_2:
## BB#3:
    ud2
```

### Answer:

(1)

由undefined.s知，main()函数在栈上存了三个局部变量a[3],b,pb，入栈顺序为b,pb,a[2],a[1],a[0]。pb的值在-8(%ebp)处，开始存放的是变量b的地址。由于数组越界，而三个局部变量在栈上是连续的，对a[3]赋值实际上是对pb赋值，最后语句`movl (%eax),%eax`时，访问的是地址为0x4的变量，而这是系统的内存空间，造成segmentation fault。

(2)

为了分析UBSan如何工作，我生成了访问a[2]的未越界代码c2.c的汇编文件ubsan_good.s，作为对照。

* ubsan_good.s

  ```assembly
  	calll	L0$pb
  L0$pb:
  	popl	%eax
  	movb	$1, %cl
  	leal	-4(%ebp), %edx
  	movl	$0, -4(%ebp)
  	movl	%edx, -8(%ebp)
  	movl	l_main.a-L0$pb(%eax), %edx
  	movl	%edx, -20(%ebp)
  	movl	(l_main.a-L0$pb)+4(%eax), %edx
  	movl	%edx, -16(%ebp)
  	movl	(l_main.a-L0$pb)+8(%eax), %edx
  	movl	%edx, -12(%ebp)
  	testb	$1, %cl
  	movl	%eax, -28(%ebp)         ## 4-byte Spill
  	jne	LBB0_2
  ## BB#1:
  	movl	-28(%ebp), %eax         ## 4-byte Reload
  	leal	l___unnamed_1-L0$pb(%eax), %ecx
  	movl	$2, %edx
  	movl	%ecx, (%esp)
  	movl	$2, 4(%esp)
  	movl	%edx, -32(%ebp)         ## 4-byte Spill
  	calll	___ubsan_handle_out_of_bounds
  LBB0_2:
  	movl	$4, -12(%ebp)
  	movl	-8(%ebp), %eax
  	movl	(%eax), %eax
  	movl	%eax, -24(%ebp)
  	addl	$40, %esp
  	popl	%ebp
  	retl
  ```

可以看出，汇编代码通过语句`testb $1, %cl`来判断是否进入`LBB0_2`标志符，从而执行访问数组之后的程序。而`%cl`的值在`ubsan_error.s`中被初始化为0，在`ubsan_good.s`中初始化为1。`ubsan_error.s`的`LBB0_2`标志符后面甚至直接省略了后面的程序，也就是说编译器在编译过程中就已经判断出存在数组越界，直接进入`__ubsan_handle_out_of_bounds`报错。这和我开始认为编译器应该是在汇编代码中判断是否数组越界的逻辑是完全不同的。

结论：编译器在编译时就判断出了数组是否越界，从而对`%cl`寄存器初始化不同的值（0或1），从而控制汇编代码进入`__ubsan_handle_out_of_bounds`模块或者接下来代码的模块，防止了数组越界的未定义行为。



### Test Directory

c1.c：访问越界数组的程序

c2.c：访问未越界数组的，和c1.c基本相同的程序

Makefile：可用于生成c1.c的未添加UBSan和已添加UBSan标志的程序和汇编代码，生成c2.c生成添加UBSan标志的汇编代码。