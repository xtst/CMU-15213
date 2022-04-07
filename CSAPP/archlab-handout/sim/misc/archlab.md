# Part A
用gcc和objdump弄成x86-64，再往y86翻译
## sum.ys
```
  .pos 0 #设置当前位置为0
  irmovq stack, %rsp #设置栈指针
  call main
  halt

        .align 8
  ele1:
        .quad 0x00a
        .quad ele2
  ele2:
        .quad 0x0b0
        .quad ele3
  ele3:
        .quad 0xc00
        .quad 0

main:
        irmovq ele1,%rdi 
        call sum_list    
        ret
sum_list:
    irmovq    $0x0,%rax
loop:
    andq   %rdi,%rdi
    je     return
    mrmovq  (%rdi),%r13
    addq    %r13,%rax
  #  irmovq $0x8, %r14
  #  addq  %r14, %rdi 
    mrmovq   0x8(%rdi),%rdi
    jmp    loop
return:    
    ret


    
  .pos 0x200 #设置栈地址
stack:

```
## ysum.ys
```
  .pos 0 #设置当前位置为0
  irmovq stack, %rsp #设置栈指针
  call main
  halt

        .align 8
  ele1:
        .quad 0x00a
        .quad ele2
  ele2:
        .quad 0x0b0
        .quad ele3
  ele3:
        .quad 0xc00
        .quad 0

main:
        irmovq ele1,%rdi 
        call rsum_list    
        ret

rsum_list:
    andq   %rdi,%rdi
    je     return1
    pushq   %rbx
    mrmovq    (%rdi),%rbx
    mrmovq    0x8(%rdi),%rdi
    call  rsum_list
    addq    %rbx,%rax
    popq    %rbx
    ret   
return1:
    irmovq    $0x0,%rax
    ret   

  .pos 0x200 #设置栈地址
stack:
```

## copy.ys
```
  .pos 0 #设置当前位置为0
  irmovq stack, %rsp #设置栈指针
  call main
  halt

.align 8
# Source block
src:
.quad 0x00a
.quad 0x0b0
.quad 0xc00
# Destination block
dest:
.quad 0x111
.quad 0x222
.quad 0x333

main:
        irmovq src,%rdi 
        irmovq dest,%rsi 
        irmovq $0x3,%rdx
        call copy_block    
        ret


copy_block:
    irmovq    $0x0,%rcx #result=rcx
loop:
    andq   %rdx,%rdx #len = rdx
    jle    return1
    mrmovq    (%rdi),%rax #rdi=src
    rmmovq    %rax,(%rsi) #rsi=dest
    xorq    %rax,%rcx #val = rax
    irmovq $0x1, %r13
    subq    %r13,%rdx
    irmovq $0x8, %r12
    addq    %r12,%rsi
    addq    %r12,%rdi
    jmp    loop
return1:    
    rrmovq    %rcx,%rax
    ret   


    
  .pos 0x200 #设置栈地址
stack:
```