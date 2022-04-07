# rotate函数
考虑分块
http://csapp.cs.cmu.edu/public/waside/waside-blocking.pdf
最后发现8*8的块最快。
注意题目已经开O2了，常见的优化手段比如循环展开没什么意义，只会更慢，毕竟人脑强不过编译器
```
char rotate_descr8[] = "rotate: 8x8 per block";
void rotate(int dim, pixel *src, pixel *dst) {
	int num = dim >> 3;
	for (int i = 0; i < num; i++) {
		for (int j = 0; j < num; j++) {
			int ifrom = (i << 3), jfrom = (j << 3);
			for (int a = 0; a < 8; a++)
				for (int b = 0; b < 8; b++) { dst[RIDX(dim - 1 - jfrom - b, ifrom + a, dim)] = src[RIDX(ifrom + a, jfrom + b, dim)]; }
		}
	}
}
```
# smooth函数