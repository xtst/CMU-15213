# Answer of all
> Border relations with Canada have never been better.
> 
> 1 2 4 8 16 32 
> 
> 2 707
> 
> 7 0
> 
> 9?>567
> 
> 4 3 2 1 6 5


# bomb1
## Answer
Border relations with Canada have never been better.
## How
```
objdum bomb -d > obj.txt
```
看汇编，打印字符串位置
```
print (char *)0x402400
```
# bomb2
## Answer
1 2 4 8 16 32
## How
看phase_2汇编代码

# bomb3
## Answer
2 707
## How
中间有一部分有未知地址，用gdb动态调试，注意答案不唯一。

关于gdb的好东西：http://csapp.cs.cmu.edu/2e/docs/gdbnotes-x86-64.pdf

# bomb4
## Answer
7 0
## How
就对着看代码即可，动手写一下，不会的就gdb动态调试。

# bomb5
## Answer
9?>567
## How
一波映射搞定

# bomb5
## Answer
4 3 2 1 6 5
## How
一波映射搞定
现在是2021年11月1日 02点42分
快肝完了，~~还好明天早八是C++~~
注意sub可以修改条件码

upd：麻了，麻中麻，蚌埠住了，我是傻逼，把mov看成lea于是完全弄错了，变得好麻烦。

如果没看错gdb弄一弄就出来了，我是伞兵。

下面是有些错误的我的注释，去vscode装个汇编插件就能有高亮
```
00000000004010f4 <phase_6>:
  4010f4:	41 56                	push   %r14
  4010f6:	41 55                	push   %r13
  4010f8:	41 54                	push   %r12
  4010fa:	55                   	push   %rbp
  4010fb:	53                   	push   %rbx
  
  4010fc:	48 83 ec 50          	sub    $0x50,%rsp
  401100:	49 89 e5             	mov    %rsp,%r13
  401103:	48 89 e6             	mov    %rsp,%rsi
  401106:	e8 51 03 00 00       	callq  40145c <read_six_numbers>
  40110b:	49 89 e6             	mov    %rsp,%r14
  40110e:	41 bc 00 00 00 00    	mov    $0x0,%r12d

  ;for i = 0 to 5 (r13 = rsp+4i)
  401114:	4c 89 ed             	mov    %r13,%rbp ;init_r13=rsp
  401117:	41 8b 45 00          	mov    0x0(%r13),%eax ; eax = a[i]
  40111b:	83 e8 01             	sub    $0x1,%eax ;eax = a[i]-1
  
  40111e:	83 f8 05             	cmp    $0x5,%eax ;
  401121:	76 05                	jbe    401128 <phase_6+0x34>
  401123:	e8 12 03 00 00       	callq  40143a <explode_bomb>
  ;if(a[i]>6) bomb!

  401128:	41 83 c4 01          	add    $0x1,%r12d ;r12d = 1
  40112c:	41 83 fc 06          	cmp    $0x6,%r12d 
  401130:	74 21                	je     401153 <phase_6+0x5f>
  ;if(i==5(r12d==6)) run!

  401132:	44 89 e3             	mov    %r12d,%ebx ;ebx = r12d

  ;for ebx = i+1 to 5
  401135:	48 63 c3             	movslq %ebx,%rax
  401138:	8b 04 84             	mov    (%rsp,%rax,4),%eax ;eax = a[ebx]

  40113b:	39 45 00             	cmp    %eax,0x0(%rbp)
  40113e:	75 05                	jne    401145 <phase_6+0x51>
  401140:	e8 f5 02 00 00       	callq  40143a <explode_bomb>
  ; if(any same in a[]) bomb!

  401145:	83 c3 01             	add    $0x1,%ebx
  401148:	83 fb 05             	cmp    $0x5,%ebx
  40114b:	7e e8                	jle    401135 <phase_6+0x41>
  ;end rbx loop //to ensure no same to a[0] in a[]

  40114d:	49 83 c5 04          	add    $0x4,%r13
  401151:	eb c1                	jmp    401114 <phase_6+0x20>
  ;end i loop

  ;the code before make sure that there's no same element in a[]


  401153:	48 8d 74 24 18       	lea    0x18(%rsp),%rsi ;rsi = &a[6]
  401158:	4c 89 f0             	mov    %r14,%rax ;rax = r14 = rbp
  40115b:	b9 07 00 00 00       	mov    $0x7,%ecx ;ecx/rcx = 7

  
  ; for i = 0 to 5
  401160:	89 ca                	mov    %ecx,%edx ;edx/rdx = exc = 7
  401162:	2b 10                	sub    (%rax),%edx ;edx -= a[i](rax)
  401164:	89 10                	mov    %edx,(%rax) ;a[i] = 7-a[i]
  401166:	48 83 c0 04          	add    $0x4,%rax ;equals to i++
  40116a:	48 39 f0             	cmp    %rsi,%rax ;
  40116d:	75 f1                	jne    401160 <phase_6+0x6c>
  ;end i loop //change every a[i] = 7-a[i]


  40116f:	be 00 00 00 00       	mov    $0x0,%esi ;rsi=esi=0
  401174:	eb 21                	jmp    401197 <phase_6+0xa3>
  
  ;for
  401176:	48 8b 52 08          	mov    0x8(%rdx),%rdx
  40117a:	83 c0 01             	add    $0x1,%eax ;eax = 2
  40117d:	39 c8                	cmp    %ecx,%eax ;eax - ecx
  40117f:	75 f5                	jne    401176 <phase_6+0x82> ;a[0] must>= 2
  401181:	eb 05                	jmp    401188 <phase_6+0x94>
  401183:	ba d0 32 60 00       	mov    $0x6032d0,%edx ;edx/rdx=0x6032d0
  401188:	48 89 54 74 20       	mov    %rdx,0x20(%rsp,%rsi,2) ;b[rsi/4]=rdx &b[0]=rsp+32
  40118d:	48 83 c6 04          	add    $0x4,%rsi
  401191:	48 83 fe 18          	cmp    $0x18,%rsi
  401195:	74 14                	je     4011ab <phase_6+0xb7>
  401197:	8b 0c 34             	mov    (%rsp,%rsi,1),%ecx ;ecx=a[rsi(/4)]
  40119a:	83 f9 01             	cmp    $0x1,%ecx 
  40119d:	7e e4                	jle    401183 <phase_6+0x8f> ;if(ecx<=1) run back
  40119f:	b8 01 00 00 00       	mov    $0x1,%eax ;eax = 1
  4011a4:	ba d0 32 60 00       	mov    $0x6032d0,%edx
  4011a9:	eb cb                	jmp    401176 <phase_6+0x82>
  ;end loop
  ;summary:
  ;for i = 0 to 5
  ;if(a[i]>=1) b[i]=6304480+(a[i]-1)*8;(after changing a[i])
  ;else b[i]=6304480; //6304480=0x6032d0


  4011ab:	48 8b 5c 24 20       	mov    0x20(%rsp),%rbx ;rbx = b[0]
  4011b0:	48 8d 44 24 28       	lea    0x28(%rsp),%rax ;rax = rsp + 40(&b[1])
  4011b5:	48 8d 74 24 50       	lea    0x50(%rsp),%rsi ;rsi = next(?rip)
  4011ba:	48 89 d9             	mov    %rbx,%rcx ;rcx = rbx = b[0]

  ;begin loop
  4011bd:	48 8b 10             	mov    (%rax),%rdx ;rdx = b[1]
  4011c0:	48 89 51 08          	mov    %rdx,0x8(%rcx) ;*(b[0]+8) = rdx;
  4011c4:	48 83 c0 08          	add    $0x8,%rax ;rax += 8
  4011c8:	48 39 f0             	cmp    %rsi,%rax ;while rax still in b[]
  4011cb:	74 05                	je     4011d2 <phase_6+0xde> ;if out b[],run!
  4011cd:	48 89 d1             	mov    %rdx,%rcx ;rcx = rdx = b[1]
  4011d0:	eb eb                	jmp    4011bd <phase_6+0xc9>
  ;end loop // summary:
  ;for i = 1 to 5
  ;*(b[i-1]+8) = b[i]
  ;note that *(b[i-1]+8) = b[i] (where after modify a[1]-a[x]=1 || a[1]==3,a[x]<=2)

  4011d2:	48 c7 42 08 00 00 00 	movq   $0x0,0x8(%rdx) ;*(b[5]+8)=0)
  4011d9:	00 
  4011da:	bd 05 00 00 00       	mov    $0x5,%ebp ;ebp/rbp=5
  4011df:	48 8b 43 08          	mov    0x8(%rbx),%rax ;rax = *(b[0]+8) = b[1]
  4011e3:	8b 00                	mov    (%rax),%eax 
  ;eax = *b[1] = *(b[x]+8) = b[x+1] (where after modify a[1]-a[x]=1 || a[1]==3,a[x]<=2)

  4011e5:	39 03                	cmp    %eax,(%rbx) ;(*b[0])-(*b[1])>=0
  4011e7:	7d 05                	jge    4011ee <phase_6+0xfa>
  4011e9:	e8 4c 02 00 00       	callq  40143a <explode_bomb>

  4011ee:	48 8b 5b 08          	mov    0x8(%rbx),%rbx ;rbx = b[next]
  4011f2:	83 ed 01             	sub    $0x1,%ebp 
  4011f5:	75 e8                	jne    4011df <phase_6+0xeb>
  ;summary:
  ;if you don't want to bomb! you should:
  ;for i = 0 to 4
  ;must (*b[i])-(*b[i+1])>=0


  4011f7:	48 83 c4 50          	add    $0x50,%rsp
  4011fb:	5b                   	pop    %rbx
  4011fc:	5d                   	pop    %rbp
  4011fd:	41 5c                	pop    %r12
  4011ff:	41 5d                	pop    %r13
  401201:	41 5e                	pop    %r14
  401203:	c3                   	retq   

```
