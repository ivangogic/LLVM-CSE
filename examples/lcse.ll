; ModuleID = 'lcse.c'
source_filename = "lcse.c"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx13.0.0"

; Function Attrs: noinline nounwind optnone ssp uwtable
define i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %x = alloca i32, align 4
  %y = alloca i32, align 4
  %z = alloca i32, align 4
  %w = alloca i32, align 4
  %dj = alloca i32, align 4
  %a1 = alloca i32, align 4
  %a2 = alloca i32, align 4
  %a = alloca [10 x i32], align 16
  %i = alloca i32, align 4
  %e1 = alloca i32, align 4
  %e2 = alloca i32, align 4
  %e3 = alloca i32, align 4
  %e4 = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  %0 = load i32, ptr %z, align 4
  %1 = load i32, ptr %w, align 4
  %2 = load i32, ptr %dj, align 4
  %mul = mul nsw i32 %1, %2
  %add = add nsw i32 %0, %mul
  store i32 %add, ptr %x, align 4
  %3 = load i32, ptr %z, align 4
  %4 = load i32, ptr %w, align 4
  %5 = load i32, ptr %dj, align 4
  %mul1 = mul nsw i32 %4, %5
  %add2 = add nsw i32 %3, %mul1
  store i32 %add2, ptr %y, align 4
  %6 = load i32, ptr %z, align 4
  %7 = load i32, ptr %w, align 4
  %add3 = add nsw i32 %6, %7
  store i32 %add3, ptr %a1, align 4
  %8 = load i32, ptr %w, align 4
  %9 = load i32, ptr %z, align 4
  %add4 = add nsw i32 %8, %9
  store i32 %add4, ptr %a2, align 4
  store i32 5, ptr %i, align 4
  %10 = load i32, ptr %i, align 4
  %idxprom = sext i32 %10 to i64
  %arrayidx = getelementptr inbounds [10 x i32], ptr %a, i64 0, i64 %idxprom
  %11 = load i32, ptr %arrayidx, align 4
  %add5 = add nsw i32 %11, 4
  store i32 %add5, ptr %e1, align 4
  %12 = load i32, ptr %i, align 4
  %idxprom6 = sext i32 %12 to i64
  %arrayidx7 = getelementptr inbounds [10 x i32], ptr %a, i64 0, i64 %idxprom6
  %13 = load i32, ptr %arrayidx7, align 4
  %div = sdiv i32 %13, 2
  store i32 %div, ptr %e2, align 4
  %14 = load i32, ptr %i, align 4
  %idxprom8 = sext i32 %14 to i64
  %arrayidx9 = getelementptr inbounds [10 x i32], ptr %a, i64 0, i64 %idxprom8
  %15 = load i32, ptr %arrayidx9, align 4
  %16 = load i32, ptr %i, align 4
  %idxprom10 = sext i32 %16 to i64
  %arrayidx11 = getelementptr inbounds [10 x i32], ptr %a, i64 0, i64 %idxprom10
  %17 = load i32, ptr %arrayidx11, align 4
  %18 = load i32, ptr %i, align 4
  %idxprom12 = sext i32 %18 to i64
  %arrayidx13 = getelementptr inbounds [10 x i32], ptr %a, i64 0, i64 %idxprom12
  %19 = load i32, ptr %arrayidx13, align 4
  %mul14 = mul nsw i32 %17, %19
  %add15 = add nsw i32 %15, %mul14
  store i32 %add15, ptr %e3, align 4
  %20 = load i32, ptr %i, align 4
  %idxprom16 = sext i32 %20 to i64
  %arrayidx17 = getelementptr inbounds [10 x i32], ptr %a, i64 0, i64 %idxprom16
  %21 = load i32, ptr %arrayidx17, align 4
  %22 = load i32, ptr %i, align 4
  %idxprom18 = sext i32 %22 to i64
  %arrayidx19 = getelementptr inbounds [10 x i32], ptr %a, i64 0, i64 %idxprom18
  %23 = load i32, ptr %arrayidx19, align 4
  %mul20 = mul nsw i32 %21, %23
  %sub = sub nsw i32 %mul20, 11
  store i32 %sub, ptr %e4, align 4
  ret i32 0
}

attributes #0 = { noinline nounwind optnone ssp uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
!4 = !{!"Homebrew clang version 16.0.5"}
