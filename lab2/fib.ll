; ModuleID = 'my_module'
source_filename = "my_module"

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
  %minus1 = add nsw i32 %n, -1
  %fib1 = call i32 @fib(i32 %minus1)
  %minus2 = add nsw i32 %n, -2
  %fib2 = call i32 @fib(i32 %minus2)
  %sum = add nsw i32 %fib1, %fib2
  br label %ret

ret:                                              ; preds = %else, %elif, %then
  %ret1 = phi i32 [ %n, %then ], [ %n, %elif ], [ %sum, %else ]
  ret i32 %ret1
}

define i32 @main() {
entry:
  %x = alloca i32
  store i32 0, i32* %x
  br label %loop

loop:                                             ; preds = %loop, %entry
  %i = phi i32 [ 0, %entry ], [ %addi, %loop ]
  %addi = add nsw i32 %i, 1
  %fibi = call i32 @fib(i32 %i)
  %xtemp = load i32, i32* %x
  %xcur = add nsw i32 %xtemp, %fibi
  store i32 %xcur, i32* %x
  %if_continue = icmp ult i32 %addi, 10
  br i1 %if_continue, label %loop, label %end

end:                                              ; preds = %loop
  %ret = load i32, i32* %x
  ret i32 %ret
}
