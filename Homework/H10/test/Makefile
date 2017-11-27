LABFLAGS = -m32 -fno-stack-protector
UBSANF = -fsanitize=bounds
CC = clang
undefasm:c1.c
	$(CC) c1.c $(LABFLAGS) -S -o undefasm.s
ubsan_error:c1.c
	$(CC) c1.c $(LABFLAGS) $(UBSANF) -S -o ubsan_error.s
ubsan_good:c2.c
	$(CC) c2.c $(LABFLAGS) $(UBSANF) -S -o ubsan_good.s
undefexe:c1.c
	$(CC) c1.c $(LABFLAGS) -o undefexe
safeexe:c1.c
	$(CC) c1.c $(LABFLAGS) $(UBSANF) -o safeexe